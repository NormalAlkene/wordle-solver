// #define DEBUG 1

#include "model.hpp"
#include "cache.hpp"
#include "priority_queue.hpp"

#include <cstdint>
#include <limits>
#include <omp.h>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <queue>
#include <ranges>
#include <fstream>
#include <iostream>

constexpr size_t WORD_LENGTH = 5u;
constexpr size_t ALPHABET_NUM = 26u;
constexpr size_t RESERVE_WORD_LIST_SIZE = 10000u;
constexpr size_t TOP_K = 5u;
constexpr size_t SHOW_CANDIDATE_NUM = 20u;

using std::span;
using std::pair;
using std::vector;
using std::string;
using std::unordered_map;
//using std::priority_queue;

using namespace wordle;

using WordT = Word<WORD_LENGTH, ALPHABET_NUM>;
using ClueT = Clue<WORD_LENGTH, ALPHABET_NUM>;
using StateT = State<WORD_LENGTH, ALPHABET_NUM>;

vector<WordT> load_words(const std::string &path)
{
    std::ifstream file(path);
    vector<WordT> ret{};

    if (!file)
        return ret;

    std::string line;
    line.reserve(WORD_LENGTH + 1);
    ret.reserve(RESERVE_WORD_LIST_SIZE);
    while (std::getline(file, line))
    {
        ret.emplace_back(std::string_view(line));
    }

    return ret;
}

priority_queue<pair<uint64_t, WordT>> pick_words(
    const StateT &state,
    span<const WordT> answer_candidates,
    span<const WordT> words,
    size_t top_k = 1)
{
    priority_queue<pair<uint64_t, WordT>> ret{};
    for (auto i = 0u; i < top_k; ++i)
    {
        ret.emplace(std::numeric_limits<uint64_t>::max(), WordT{});
    }
#pragma omp parallel 
    {
        decltype(ret) local_ret(ret);
        for (auto i = 0u; i < top_k; ++i)
        {
            local_ret.emplace(std::numeric_limits<uint64_t>::max(), WordT{});
        }
#pragma omp for schedule(static)
        for (const auto &guess : words)
        {
            unordered_map<ClueT, uint64_t> clues{};
            uint64_t expectation = 0;
            for (const auto &answer : answer_candidates)
            {
                ++clues[Clue(guess, answer)];
            }
            for (const auto &[clue, weight] : clues)
            {
                auto new_state = state & State(clue);
                size_t valid_count = 0;
                for (const auto &it : words)
                {
                    valid_count += new_state.check(it);
                }
                expectation += valid_count * weight;
            }
            if (expectation > 0)
            {
                local_ret.pushpop(std::make_pair(expectation, guess));
            }
        }
#pragma omp critical // join
        for (auto&& it : local_ret.data())
        {
            ret.pushpop(std::move(it));
        }
    }
    return ret;
}

int main(int argc, char *argv[])
{
    using std::cin, std::cout, std::cerr, std::endl;
    cerr << "Loading word list..." << endl;
    string word_list_path = "./data/word_list.txt";
    if (argc > 1)
    {
        word_list_path = argv[1];
    }

    auto words = load_words(word_list_path);
    auto answer_candidates = vector(words);

    StateT state{};
    for (;;)
    {
        // cerr << "Current count answer cadidates is: " << answer_candidates.size() << endl;
        auto suggestions = pick_words(state, answer_candidates, words, TOP_K);
        cerr << "Top " << TOP_K << " guess candidates:" << endl;
        string guess_word;
        while (!suggestions.is_empty())
        {
            const auto &it = suggestions.top();
            cerr << it.second.str() << ": " << static_cast<double>(it.first) / answer_candidates.size() << endl;
            guess_word = it.second.str();
            suggestions.pop();
        }
        cerr << "Suggested word: ";
        cout << guess_word << endl;

        string temp;
        cerr << "Guess or clue (if suggested word used): ";
        cin >> temp;
        if (isalpha(temp[0]))
        {
            guess_word = std::move(temp);
            cerr << " Clue: ";
            cin >> temp;
        }
        cerr << "Your guess is: " << guess_word << '(' << answer_candidates.size() << ')' << endl;
        auto clue = Clue(WordT(guess_word));
        std::array<ClueType, WORD_LENGTH> clue_str;
        for (auto i = 0u; i < WORD_LENGTH; ++i)
        {
            switch (temp[i]) {
                case '0':
                    clue_str[i] = CLUE_GRAY;
                    break;
                case '1':
                    clue_str[i] = CLUE_YELLOW;
                    break;
                case '2':
                    clue_str[i] = CLUE_GREEN;
                    break;
                default:
                    cerr << "Warning: invalid clue char " << temp[i] << ".\n";
                    clue_str[i] = CLUE_GRAY;
            }
        }
        clue.set(clue_str);
        state &= State(clue);
        answer_candidates = 
            answer_candidates
            | std::views::filter( [&state](auto x){return state.check(x);})
            | std::ranges::to<vector>();

        cerr << "Candidates (total " << answer_candidates.size() << "): ";
        auto i = 0u;
        for (const auto &it : answer_candidates)
        {
            if (i >= SHOW_CANDIDATE_NUM)
                break;
            cerr << it.str() << ' ';
            ++i;
        }
        cerr << endl;
        if (answer_candidates.size() == 1)
        {
            cerr << "Answer: " << answer_candidates[0].str() << endl;
            break;
        }
        else if (answer_candidates.size() == 0)
        {
            cerr << "No answer candidate left!" << endl;
#ifdef DEBUG
            cerr << "Current state is: \n\t_possible[][]: \n";
            cerr << "  ";
            for (auto i = 0; i < ALPHABET_NUM; ++i)
            {
                cerr << static_cast<char>('a' + i) << ' ';
            }
            cerr << endl;
            auto i = 1;
            for (const auto &it : state.get_possible_matrix())
            {
                cerr << i << ' ';
                for (auto jt : it)
                {
                    cerr << static_cast<uint32_t>(jt) << ' ';
                }
                cerr << endl;
                ++i;
            }
            cerr << endl;
            for (auto i = 0; i < ALPHABET_NUM; ++i)
            {
                cerr << static_cast<char>('a' + i) << ' ';
            }
            cerr << endl;
            for (const auto &it : state.get_min_count())
            {
                cerr << it << ' ';
            }
            cerr << endl;
            for (const auto &it : state.get_max_count())
            {
                cerr << it << ' ';
            }
            cerr << endl;
#endif
            break;
        }
    }
    return 0;
}
