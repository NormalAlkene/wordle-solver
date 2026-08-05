#ifndef WORDLE_CACHE_HPP
#define WORDLE_CACHE_HPP 1

#include "common.hpp"
#include "model.hpp"

#include <filesystem>
#include <vector>

namespace wordle
{
    bool load_cache(
        const std::filesystem::path &path,
        std::vector<Word<WORD_LENGTH>> &word_list,
        std::vector<WordWeight<WORD_LENGTH>> &answer_list,
        std::vector<float> &scores);
        // std::vector<State<WORD_LENGTH>> &state_list);

    bool store_cache(
        const std::filesystem::path &path,
        const std::vector<WordWeight<WORD_LENGTH>> &word_list,
        const std::vector<float> &scores);
        // const std::vector<State<WORD_LENGTH>> &state_list);
} /* namespace wordle */


#endif /* ifndef WORDLE_CACHE_HPP */
