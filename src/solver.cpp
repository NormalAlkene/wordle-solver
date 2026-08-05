// #define DEBUG 1

#include "common.hpp"
#include "model.hpp"
#include "cache.hpp"
#include "priority_queue.hpp"

#include <cstddef>
#include <limits>
#include <omp.h>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <iostream>

constexpr size_t TOP_K = 5u;
constexpr size_t SHOW_CANDIDATE_NUM = 20u;

using std::span;
using std::pair;
using std::vector;
using std::string;
using std::unordered_map;

using namespace wordle;

using WordT = Word<WORD_LENGTH>;
using WordWeightT = WordWeight<WORD_LENGTH>;
using ClueT = Clue<WORD_LENGTH>;
using StateT = State<WORD_LENGTH>;

priority_queue<pair<float, WordT>> pick_words(
    const StateT &state,
    span<const WordWeightT> answer_candidates,
    span<const WordT> words,
    size_t top_k = 1)
{
    float sum_weight = 0.0f;
    for (const auto &it : answer_candidates)
    {
        sum_weight += it.weight;
    }

    priority_queue<pair<float, WordT>> ret{};
    for (auto i = 0u; i < top_k; ++i)
    {
        ret.emplace(std::numeric_limits<float>::infinity(), WordT{});
    }
#pragma omp parallel 
    {
        decltype(ret) local_ret(ret);
        for (auto i = 0u; i < top_k; ++i)
        {
            local_ret.emplace(std::numeric_limits<float>::infinity(), WordT{});
        }
#pragma omp for schedule(static)
        for (const auto &guess : words)
        {
            unordered_map<StateT, float> states;
            float expectation = 0;
            for (const auto &answer : answer_candidates)
            {
                states[State(guess, answer.word)] += answer.weight;
            }
            for (const auto &[state, state_weight] : states)
            {
                auto new_state = state & state;
                float valid_weight = 0;
                for (const auto &[word, answer_weight] : answer_candidates)
                {
                    valid_weight += new_state.check(word) * answer_weight;
                }
                expectation += valid_weight * state_weight / sum_weight;
            }
            if (expectation > EPSILON)
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
    string word_list_path = "./data/cache.dat";
    if (argc > 1)
    {
        word_list_path = argv[1];
    }
    else
    {
        cerr << "Cache file is not specified, using default: " << word_list_path << '.' << endl;
    }

    vector<WordT> words;
    vector<WordWeightT> answer_candidates;
    if (!load_cache(word_list_path, words, answer_candidates))
    {
        cerr << "Invalid cache file: " << word_list_path << '.' << endl;
        return 1;
    }

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
            cerr << it.second.str() << ": " << it.first << endl;
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
            | std::views::filter( [&state](auto x){return state.check(x.word);})
            | std::ranges::to<vector>();

        cerr << "Candidates (total " << answer_candidates.size() << "): ";
        auto i = 0u;
        for (const auto &it : answer_candidates)
        {
            if (i >= SHOW_CANDIDATE_NUM)
                break;
            cerr << it.word.str() << ' ';
            ++i;
        }
        cerr << endl;
        if (answer_candidates.size() == 1)
        {
            cerr << "Answer: " << answer_candidates[0].word.str() << endl;
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
