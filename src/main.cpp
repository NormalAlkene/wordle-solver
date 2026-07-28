#include "model.hpp"

#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <queue>

constexpr size_t WORD_LENGTH = 5u;
constexpr size_t ALPHABET_NUM = 26u;
constexpr size_t RESERVE_WORD_LIST_SIZE = 10000u;
constexpr uint32_t TOP_K = 10u;

using std::vector;
using std::string;
using std::unordered_set;
using std::unordered_map;
using std::priority_queue;

using WordT = Word<WORD_LENGTH, ALPHABET_NUM>;
using ClueT = Clue<WORD_LENGTH, ALPHABET_NUM>;
using StateT = State<WORD_LENGTH, ALPHABET_NUM>;

vector<WordT> load_words(const char * path)
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

int main(void)
{
    StateT s{};
    return 0;
}
