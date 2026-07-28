#ifndef MODEL_H
#define MODEL_H

#include <cstdint>
#include <functional>
#include <span>
#include <array>
#include <string>

template <size_t LENGTH, size_t ALPHABET_NUM>
class Word
{
    private:
        std::array<uint32_t, LENGTH> _word{};
        std::array<uint32_t, ALPHABET_NUM> _bincount{};

        void _calc_bincount(void) noexcept;

    public:
        explicit Word() noexcept = default;
        explicit Word(std::string_view str) noexcept;
        // explicit Word(std::span<const decltype(_word[0]), LENGTH> word) noexcept;
        std::span<const uint32_t, ALPHABET_NUM> bincount(void) const noexcept;
        std::string str() const noexcept;
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
        std::array<ClueType, LENGTH> _clue{};

    public:
        explicit Clue(const Word<LENGTH, ALPHABET_NUM> &word) noexcept;
        explicit Clue(const Word<LENGTH, ALPHABET_NUM> &guess, const Word<LENGTH, ALPHABET_NUM> &answer) noexcept;

        void set(std::span<ClueType, LENGTH> value) noexcept;
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
        std::array<std::array<bool, ALPHABET_NUM>, LENGTH> _possible{};
        std::array<size_t, ALPHABET_NUM>_min_count{};
        std::array<size_t, ALPHABET_NUM>_max_count{};
    public:
        explicit State(void) noexcept;
        explicit State(const Clue<LENGTH, ALPHABET_NUM> &clue) noexcept;
        bool check(const Word<LENGTH, ALPHABET_NUM> &word) const noexcept;
        State operator&(const State<LENGTH, ALPHABET_NUM> &other) const noexcept;
        State &operator&=(const State<LENGTH, ALPHABET_NUM> &other) noexcept;
        bool operator==(const State<LENGTH, ALPHABET_NUM> &other) const noexcept = default;
};

template <size_t LENGTH, size_t ALPHABET_NUM>
void Word<LENGTH, ALPHABET_NUM>::_calc_bincount(void) noexcept
{
    this->_bincount.fill(0);
    for (const auto &it : this->_word)
    {
        ++this->_bincount[it];
    }
}

template <size_t LENGTH, size_t ALPHABET_NUM>
Word<LENGTH, ALPHABET_NUM>::Word(std::string_view str) noexcept
{
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            this->_word[i] = str[i] - 'a';
        else if (str[i] >= 'A' && str[i] <= 'Z')
            this->_word[i] = str[i] - 'A';
        else
            this->_word[i] = 0u;
    }
    this->_calc_bincount();
}

/*
template <size_t LENGTH, size_t ALPHABET_NUM>
Word<LENGTH, ALPHABET_NUM>::Word(std::span<const decltype(_word[0]), LENGTH> word) noexcept
{
    this->_word = word;
    this->_calc_bincount();
}
*/

template <size_t LENGTH, size_t ALPHABET_NUM>
std::span<const uint32_t, ALPHABET_NUM> Word<LENGTH, ALPHABET_NUM>::bincount(void) const noexcept
{
    return this->_bincount;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
std::string Word<LENGTH, ALPHABET_NUM>::str() const noexcept
{
    auto ret = std::string(LENGTH, 0u);
    for (auto i = 0u; i < LENGTH; ++i)
    {
        ret[i] = this->_word[i] + 'a';
    }
    return ret;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
bool Word<LENGTH, ALPHABET_NUM>::operator==(const Word &other) const noexcept
{
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (this->_word[i] != other._word[i])
            return false;
    }
    return true;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
std::strong_ordering Word<LENGTH, ALPHABET_NUM>::operator<=>(const Word &other) const noexcept
{
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (auto cmp = (this->_word[i] <=> other._word[i]); cmp != 0)
            return cmp;
    }
    return std::strong_ordering::equal;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
uint32_t Word<LENGTH, ALPHABET_NUM>::operator[](size_t index) const noexcept
{
    return this->_word[index];
}

template <size_t LENGTH, size_t ALPHABET_NUM>
uint32_t& Word<LENGTH, ALPHABET_NUM>::operator[](size_t index) noexcept
{
    return this->_word[index];
}

template <size_t LENGTH, size_t ALPHABET_NUM>
size_t std::hash<Word<LENGTH, ALPHABET_NUM>>::operator()(const Word<LENGTH, ALPHABET_NUM> &object) const
{
    size_t ret = 0u;
    size_t coefficient = 1u;
    for (auto i = 0u; i < LENGTH; ++i)
    {
        ret += coefficient * object[i];
        coefficient *= ALPHABET_NUM;
    }
    return ret;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
Clue<LENGTH, ALPHABET_NUM>::Clue(const Word<LENGTH, ALPHABET_NUM> &word) noexcept
{
    this->_word = word;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
Clue<LENGTH, ALPHABET_NUM>::Clue(
    const Word<LENGTH, ALPHABET_NUM> &guess,
    const Word<LENGTH, ALPHABET_NUM> &answer) noexcept
{
    this->_word = guess;
    this->_clue.fill(ClueType::CLUE_GRAY);

    // 1. Green
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (guess[i] == answer[i])
        {
            this->_clue[i] = ClueType::CLUE_GREEN;
        }
    }

    // 2. Yellow
    std::array<uint32_t, ALPHABET_NUM> temp_bincount;
    std::ranges::copy(answer.bincount(), temp_bincount.begin());
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (this->_clue[i] == CLUE_GRAY && temp_bincount[guess[i]] > 0)
        {
            this->_clue[i] = CLUE_YELLOW;
            --temp_bincount[guess[i]];
        }
    }
}

template <size_t LENGTH, size_t ALPHABET_NUM>
void Clue<LENGTH, ALPHABET_NUM>::set(std::span<ClueType, LENGTH> value) noexcept
{
    std::ranges::copy(value, this->_clue.begin());
}

template <size_t LENGTH, size_t ALPHABET_NUM>
const Word<LENGTH, ALPHABET_NUM> & Clue<LENGTH, ALPHABET_NUM>::get_word(void) const noexcept
{
    return this->_word;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
ClueType Clue<LENGTH, ALPHABET_NUM>::operator[](size_t index) const noexcept
{
    return this->_clue[index];
}

template <size_t LENGTH, size_t ALPHABET_NUM>
ClueType& Clue<LENGTH, ALPHABET_NUM>::operator[](size_t index) noexcept
{
    return this->_clue[index];
}

template <size_t LENGTH, size_t ALPHABET_NUM>
size_t std::hash<Clue<LENGTH, ALPHABET_NUM>>::operator()(const Clue<LENGTH, ALPHABET_NUM> &object) const
{
    size_t ret = 0u;
    size_t coefficient = 1u;
    for (auto i = 0u; i < LENGTH; ++i)
    {
        ret += coefficient * ((size_t)object[i] + 1) * object.get_word()[i];
        coefficient *= ALPHABET_NUM * 3;
    }
    return ret;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
State<LENGTH, ALPHABET_NUM>::State(void) noexcept
{
    for (auto &it : this->_possible)
    {
        it.fill(true);
    }
    this->_min_count.fill(0);
    this->_max_count.fill(LENGTH);
}

template <size_t LENGTH, size_t ALPHABET_NUM>
State<LENGTH, ALPHABET_NUM>::State(const Clue<LENGTH, ALPHABET_NUM> &clue) noexcept : State()
{
    // Green
    for (auto i = 0u; i < LENGTH; ++i)
    {
        auto cur_letter = clue.get_word()[i];
        if (clue[i] == CLUE_GREEN)
        {
            this->_possible[i].fill(false);
            this->_possible[i][cur_letter] = true;
            ++this->_min_count[cur_letter];
            for (auto &it : this->_max_count)
            {
                --it;
            }
            ++this->_max_count[cur_letter];
        }
    }

    // Yellow & gray
    for (auto i = 0u; i < LENGTH; ++i)
    {
        auto cur_letter = clue.get_word()[i];
        switch (clue[i]) {
            case ClueType::CLUE_YELLOW:
                this->_possible[i][cur_letter] = false;
                ++this->_min_count[cur_letter];
                for (auto &it : this->_max_count)
                {
                    --it;
                }
                ++this->_max_count[cur_letter];
                break;
            case ClueType::CLUE_GRAY:
                this->_possible[i][cur_letter] = false;
                this->_max_count[cur_letter] = this->_min_count[cur_letter];
                break;
        }
    }
    // TODO: is this correct?
}

template <size_t LENGTH, size_t ALPHABET_NUM>
bool State<LENGTH, ALPHABET_NUM>::check(const Word<LENGTH, ALPHABET_NUM> &word) const noexcept
{
    // 1. Possibility
    for (auto i = 0u; i < LENGTH; ++i)
    {
        if (!this->_possible[i][word[i]])
            return false;
    }

    // 2. Count
    for (auto i = 0u; i < ALPHABET_NUM; ++i)
    {
        if (word.bincount()[i] < this->_min_count[i] || word.bincount()[i] > this->_max_count[i])
            return false;
    }
    return true;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
State<LENGTH, ALPHABET_NUM> State<LENGTH, ALPHABET_NUM>::operator&(const State<LENGTH, ALPHABET_NUM> &other) const noexcept
{
    State<LENGTH, ALPHABET_NUM> ret{*this};
    ret &= other;
    return ret;
}

template <size_t LENGTH, size_t ALPHABET_NUM>
State<LENGTH, ALPHABET_NUM> &State<LENGTH, ALPHABET_NUM>::operator&=(const State<LENGTH, ALPHABET_NUM> &other) noexcept
{
    for (auto i = 0u; i < LENGTH; ++i)
    {
        for (auto j = 0u; j < ALPHABET_NUM; ++j)
        {
            this->_possible[i][j] = this->_possible[i][j] && other._possible[i][j];
        }
    }
    for (auto i = 0u; i < ALPHABET_NUM; ++i)
    {
        this->_min_count[i] = std::max(this->_min_count[i], other._min_count[i]);
    }
    for (auto i = 0u; i < ALPHABET_NUM; ++i)
    {
        this->_max_count[i] = std::min(this->_max_count[i], other._max_count[i]);
    }

    auto sum = 0u;
    for (const auto &it : this->_min_count)
    {
        sum += it;
    }
    for (auto i = 0u; i < ALPHABET_NUM; ++i)
    {
        this->_max_count[i] = LENGTH - sum + this->_min_count[i];
    }
    // TODO: symmetric for min?
    return *this;
}

#endif /* MODEL_H */

