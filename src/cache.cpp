#include "common.hpp"
#include "cache.hpp"
#include "model.hpp"

#include <filesystem>
#include <fstream>
#include <limits>
#include <type_traits>

constexpr uint32_t HEADER = 0xCA005357u; // 'W' 'S'
constexpr uint16_t VERSION = 2u;

using namespace wordle;
using std::ios;
using std::vector;

using WordT = Word<WORD_LENGTH>;
using WordWeightT = WordWeight<WORD_LENGTH>;
using ScoreT = float;
// using StateT = State<WORD_LENGTH>;
static_assert(std::is_trivially_copyable_v<WordT>);
static_assert(std::is_trivially_copyable_v<WordWeightT>);

#pragma pack (push, 1)
struct FileHead
{
    uint32_t header;
    uint16_t version;
    uint8_t alphabet_num;
    uint8_t word_length;
    // The size of word list cache, in bytes.
    uint32_t raw_list_size;
    uint32_t score_list_size;
};
#pragma pack (pop)

bool wordle::load_cache(
    const std::filesystem::path &path,
    vector<WordT> &word_list,
    vector<WordWeightT> &answer_list,
    vector<ScoreT> &score_list)
    // vector<StateT> &state_list)
{
    auto file_size = std::filesystem::file_size(path);
    if (file_size < sizeof(FileHead))
        return false;

    FileHead head;
    std::ifstream file(path, ios::binary);
    file.read(reinterpret_cast<char *>(&head), sizeof(head));

    auto raw_list_len = head.raw_list_size / sizeof(WordWeightT);
    // auto state_list_len = head.state_list_size / sizeof(StateT);

    if ((!file) ||
        (file_size < sizeof(head) + head.raw_list_size) || // + head.state_list_size) ||
        (head.header != HEADER) ||
        (head.version != VERSION) ||
        (head.alphabet_num != ALPHABET_NUM) ||
        (head.word_length != WORD_LENGTH) ||
        (head.raw_list_size % sizeof(WordWeightT)) ||
        (head.score_list_size % sizeof(ScoreT)) )
        return false;

    vector<WordWeightT> raw_list;
    raw_list.resize(raw_list_len);
    file.read(reinterpret_cast<char *>(raw_list.data()), head.raw_list_size);
    if (!file)
        return false;

    word_list.clear();
    answer_list.clear();
    word_list.reserve(raw_list.size());
    for (const auto& it : raw_list)
    {
        word_list.emplace_back(it.word);
        if (it.weight > EPSILON)
        {
            answer_list.emplace_back(it);
        }
    }

    score_list.clear();
    score_list.resize(raw_list_len);
    file.read(reinterpret_cast<char *>(score_list.data()), head.score_list_size);
    if (!file)
        return false;

    if (word_list.size() != score_list.size())
        return false;

    return true;
}

bool wordle::store_cache(
    const std::filesystem::path &path,
    const vector<WordWeightT> &raw_list,
    const vector<ScoreT> &score_list)
    // const vector<StateT> &state_list)
{
    std::ofstream file(path, ios::binary | ios::trunc);
    if (!file)
        return false;

    size_t raw_list_size = raw_list.size() * sizeof(WordWeightT);
    size_t score_list_size = score_list.size() * sizeof(ScoreT);

    // size_t state_list_size = state_list.size() * sizeof(StateT);
    if ((raw_list_size > std::numeric_limits<uint32_t>::max()) ||
        (score_list_size > std::numeric_limits<uint32_t>::max()) ||
        (raw_list.size() != score_list.size()))
        return false;

    FileHead head{
        .header = HEADER,
        .version = VERSION,
        .alphabet_num = ALPHABET_NUM,
        .word_length = WORD_LENGTH,
        .raw_list_size = static_cast<uint32_t>(raw_list_size),
        .score_list_size = static_cast<uint32_t>(score_list_size),
        // .state_list_size = static_cast<uint32_t>(state_list_size),
    };
    file.write(reinterpret_cast<const char *>(&head), sizeof(head));
    if (!file)
        return false;

    file.write(reinterpret_cast<const char *>(raw_list.data()), raw_list_size); 
    if (!file)
        return false;

    file.write(reinterpret_cast<const char *>(score_list.data()), score_list_size); 
    if (!file)
        return false;

    return true;
}
