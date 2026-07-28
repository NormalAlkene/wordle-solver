#include "model.hpp"
// #define DEBUG 1

#include <omp.h>
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
constexpr uint32_t TOP_K = 10u;

using std::span;
using std::pair;
using std::vector;
using std::string;
using std::unordered_map;
using std::priority_queue;

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

priority_queue<pair<size_t, WordT>> pick_words(
    const StateT &state,
    span<const WordT> answer_candidates,
    span<const WordT> words,
    size_t top_k = 1)
{
    priority_queue<pair<size_t, WordT>> ret{};
    for (auto i = 0u; i < top_k; ++i)
    {
        ret.emplace(answer_candidates.size() * answer_candidates.size(), WordT{});
    }
#pragma omp parallel for 
    for (const auto &guess : words)
    {
        unordered_map<ClueT, size_t> clues{};
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
#pragma omp critical
        {
            ret.emplace(expectation, guess);
            ret.pop();
        }
#ifdef DEBUG
        std::cerr << guess.str() << ": " << expectation << std::endl;
#endif // DEBUG
    }
    return ret;
}

int main(int argc, char *argv[])
{
    using std::cin, std::cout, std::cerr, std::endl;
    cout << "Loading word list..." << endl;
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
        cout << "Current count answer cadidates is: " << answer_candidates.size() << endl;
        cout << "Calculating..." << endl;
        auto suggestions = pick_words(state, answer_candidates, words, TOP_K);
        cout << "Top " << TOP_K << " guess candidates:" << endl;
        while (!suggestions.empty())
        {
            const auto &it = suggestions.top();
            cout << it.second.str() << ": " << (double)it.first / answer_candidates.size() << endl;
            suggestions.pop();
        }
        string temp;
        cout << "Guess: ";
        cin >> temp;
        auto cur_clue = Clue(WordT(temp));
        cout << " Clue: ";
        cin >> temp;
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
                    cout << "Warning: invalid clue char " << temp[i] << ".\n";
                    clue_str[i] = CLUE_GRAY;
            }
        }
        cur_clue.set(clue_str);
        state &= State(cur_clue);
        answer_candidates = 
            answer_candidates
            | std::views::filter( [&state](auto x){return state.check(x);})
            | std::ranges::to<vector>();
        if (answer_candidates.size() == 1)
        {
            cout << "Answer: " << answer_candidates[0].str() << endl;
            break;
        }
        else if (answer_candidates.size() == 0)
        {
            cout << "No answer candidate left!" << endl;
            break;
        }
    }
    return 0;
}
