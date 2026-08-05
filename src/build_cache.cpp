#include "common.hpp"
#include "model.hpp"
#include "cache.hpp"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <string_view>
#include <iostream>

using namespace wordle;
using std::vector;
using std::string;

using WordT = Word<WORD_LENGTH>;
using WordWeightT = WordWeight<WORD_LENGTH>;
using ClueT = Clue<WORD_LENGTH>;
using StateT = State<WORD_LENGTH>;

constexpr size_t RESERVE_WORD_LIST_SIZE = 10000u;

bool load_words(const std::filesystem::path &path, vector<WordWeightT> &ret)
{
    std::ifstream file(path);

    if (!file)
        return false;

    string line;
    string word;
    word.reserve(WORD_LENGTH + 1);
    ret.clear();
    ret.reserve(RESERVE_WORD_LIST_SIZE);
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') [[unlikely]]
            continue;

        std::istringstream iss(line);
        decltype(WordWeightT::weight) weight;
        iss >> word >> weight;

        if (!iss) [[unlikely]]
            return false;

        ret.emplace_back(std::string_view(word), weight);
    }

    return true;
}

int main(int argc, char *argv[])
{
    using std::cin, std::cout, std::cerr, std::endl;
    if (argc > 3)
    {
        cerr << "Usage: " << argv[0] << " /path/to/word_list /path/to/cache." << endl;
        return 1;
    }

    std::filesystem::path word_list_path("./data/word_list.txt");
    std::filesystem::path cache_path("./data/cache.dat");
    if (argc >= 2)
    {
        word_list_path = argv[1];
    }
    if (argc >= 3)
    {
        cache_path = argv[2];
    }

    cerr << "Word list path: " << word_list_path << endl;
    cerr << "Cache file path: " << cache_path << endl;
    vector<WordWeightT> raw_list;

    if (!load_words(word_list_path, raw_list))
    {
        cerr << "Invalid word list file: " << word_list_path << '.' << endl;
        return 1;
    }

    if (std::filesystem::exists(cache_path))
    {
        cerr << "The cache file " << cache_path << " already exists. Overwrite? [y/N]";
        char choice;
        cin >> choice;
        if ((choice != 'y') && (choice != 'Y'))
        {
            return 0;
        }
    }

    if (!wordle::store_cache(cache_path, raw_list))
    {
        cerr << "An error occurred when writing cache!" << endl;
    }

    cerr << "Done." << endl;

    return 0;
}

