#ifndef MODEL_H
#define MODEL_H

#include <cstddef>
#include <cstdint>
#include <functional>

template <size_t LENGTH, size_t ALPHABET_NUM>
class Word
{
    private:
        uint32_t _word[LENGTH]{};
        uint32_t _bincount[ALPHABET_NUM]{};

        void _calc_bincount(void) noexcept;

    public:
        explicit Word(const char *str) noexcept;
        explicit Word(const uint32_t *word) noexcept;
        const uint32_t *bincount(void) const noexcept;
        bool operator==(const Word<LENGTH, ALPHABET_NUM> &other) const noexcept;
        std::strong_ordering operator<=>(const Word<LENGTH, ALPHABET_NUM> &other) const noexcept;
        uint32_t operator[](size_t index) const noexcept;
        uint32_t& operator[](size_t index) noexcept;
};

template <size_t LENGTH, size_t ALPHABET_NUM>
struct std::hash<Word<LENGTH, ALPHABET_NUM>>
{
    size_t operator()(const Word<LENGTH, ALPHABET_NUM> &object) const;
};

enum ClueType
{
    CLUE_GRAY = 0,
    CLUE_YELLOW,
    CLUE_GREEN,
};

template <size_t LENGTH, size_t ALPHABET_NUM>
class Clue
{
    private:
        Word<LENGTH, ALPHABET_NUM> _word{};
        ClueType _clue[LENGTH]{};

    public:
        explicit Clue(const Word<LENGTH, ALPHABET_NUM> &word) noexcept;
        explicit Clue(const Word<LENGTH, ALPHABET_NUM> &guess, const Word<LENGTH, ALPHABET_NUM> &answer) noexcept;

        void set(const ClueType *value) noexcept;
        const Word<LENGTH, ALPHABET_NUM> &get_word(void) const noexcept;
        bool operator==(const Clue<LENGTH, ALPHABET_NUM> &other) const noexcept = default;
        ClueType operator[](size_t index) const noexcept;
        ClueType& operator[](size_t index) noexcept;
};

template <size_t LENGTH, size_t ALPHABET_NUM>
struct std::hash<Clue<LENGTH, ALPHABET_NUM>>
{
    size_t operator()(const Clue<LENGTH, ALPHABET_NUM> &object) const;
};

template <size_t LENGTH, size_t ALPHABET_NUM>
class State
{
    private:
        bool _possible[LENGTH][ALPHABET_NUM]{};
        size_t _min_count[ALPHABET_NUM]{};
        size_t _max_count[ALPHABET_NUM]{};
    public:
        explicit State(void) noexcept;
        explicit State(const Clue<LENGTH, ALPHABET_NUM> &clue) noexcept;
        bool check(const Word<LENGTH, ALPHABET_NUM> &word) const noexcept;
        State operator&(const State<LENGTH, ALPHABET_NUM> &other) const noexcept;
        State &operator&=(const State<LENGTH, ALPHABET_NUM> &other) noexcept;
        bool operator==(const State<LENGTH, ALPHABET_NUM> &other) const noexcept = default;
};

#endif /* MODEL_H */

