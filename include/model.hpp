#ifndef WORDLE_MODEL_HPP
#define WORDLE_MODEL_HPP

#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <array>
#include <string>
#include <bitset>
#include <boost/container_hash/hash.hpp>

namespace wordle{

    inline constexpr auto ALPHABET_NUM = 26u;
    template <size_t LENGTH>
    concept ValidLength = (LENGTH < std::numeric_limits<uint8_t>::max());

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    class Word
    {
        private:
            std::array<uint8_t, LENGTH> _word{};
            std::array<uint8_t, ALPHABET_NUM> _bincount{};

            void _calc_bincount(void) noexcept;

        public:
            explicit Word() noexcept = default;
            Word(std::string_view str) noexcept;
            auto bincount(void) const noexcept -> decltype(_bincount)
            {
                return this->_bincount;
            }

            std::string str() const noexcept;
            bool operator==(const Word<LENGTH> &other) const noexcept;
            std::strong_ordering operator<=>(const Word<LENGTH> &other) const noexcept;
            uint8_t operator[](size_t index) const noexcept
            {
                return this->_word[index];
            }
    };

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    struct WordWeight
    {
        Word<LENGTH> word;
        float weight;
    };

    enum ClueType
    {
        CLUE_GRAY = 0,
        CLUE_YELLOW,
        CLUE_GREEN,
    };

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    class Clue
    {
        private:
            Word<LENGTH> _word{};
            std::array<ClueType, LENGTH> _clue{};

        public:
            explicit Clue(const Word<LENGTH> &word) noexcept;
            explicit Clue(const Word<LENGTH> &guess, const Word<LENGTH> &answer) noexcept;

            void set(std::span<ClueType, LENGTH> value) noexcept;
            const Word<LENGTH> &get_word(void) const noexcept;
            bool operator==(const Clue<LENGTH> &other) const noexcept = default;
            ClueType operator[](size_t index) const noexcept
            {
                return this->_clue[index];
            }
            ClueType& operator[](size_t index) noexcept
            {
                return this->_clue[index];
            }

    };

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    class State
    {
        private:
            std::bitset<ALPHABET_NUM * LENGTH> _possible{};
            std::array<uint8_t, ALPHABET_NUM> _min_count{};
            std::array<uint8_t, ALPHABET_NUM> _max_count{};
            static constexpr std::bitset<ALPHABET_NUM * LENGTH> _get_word_mask(uint8_t word_pos);
        public:
            explicit State(void) noexcept;
            explicit State(const Clue<LENGTH> &clue) noexcept;
            explicit State(const Word<LENGTH> &guess, const Word<LENGTH> &answer) noexcept;
            bool check(const Word<LENGTH> &word) const noexcept;
            auto get_possible_matrix() const noexcept -> const decltype(_possible)&
            {
                return _possible;
            }

            auto get_min_count() const noexcept -> const decltype(_min_count)&
            {
                return _min_count;
            }

            auto get_max_count() const noexcept -> const decltype(_max_count)&
            {
                return _max_count;
            }

            State operator&(const State<LENGTH> &other) const noexcept;
            State &operator&=(const State<LENGTH> &other) noexcept;
            bool operator==(const State<LENGTH> &other) const noexcept = default;
    };

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    void Word<LENGTH>::_calc_bincount(void) noexcept
    {
        this->_bincount.fill(0);
        for (const auto &it : this->_word)
        {
            ++this->_bincount[it];
        }
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    Word<LENGTH>::Word(std::string_view str) noexcept
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

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    std::string Word<LENGTH>::str() const noexcept
    {
        auto ret = std::string(LENGTH, 0u);
        for (auto i = 0u; i < LENGTH; ++i)
        {
            ret[i] = this->_word[i] + 'a';
        }
        return ret;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    bool Word<LENGTH>::operator==(const Word &other) const noexcept
    {
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (this->_word[i] != other._word[i])
                return false;
        }
        return true;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    std::strong_ordering Word<LENGTH>::operator<=>(const Word &other) const noexcept
    {
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (auto cmp = (this->_word[i] <=> other._word[i]); cmp != 0)
                return cmp;
        }
        return std::strong_ordering::equal;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    Clue<LENGTH>::Clue(const Word<LENGTH> &word) noexcept
    {
        this->_word = word;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    Clue<LENGTH>::Clue(
        const Word<LENGTH> &guess,
        const Word<LENGTH> &answer) noexcept
    {
        this->_word = guess;
        this->_clue.fill(ClueType::CLUE_GRAY);
        std::array<uint8_t, ALPHABET_NUM> temp_bincount;
        std::ranges::copy(answer.bincount(), temp_bincount.begin());

        // 1. Green
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (guess[i] == answer[i])
            {
                this->_clue[i] = ClueType::CLUE_GREEN;
                --temp_bincount[guess[i]];
            }
        }

        // 2. Yellow
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (this->_clue[i] == CLUE_GRAY && temp_bincount[guess[i]] > 0)
            {
                this->_clue[i] = CLUE_YELLOW;
                --temp_bincount[guess[i]];
            }
        }
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    void Clue<LENGTH>::set(std::span<ClueType, LENGTH> value) noexcept
    {
        std::ranges::copy(value, this->_clue.begin());
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    const Word<LENGTH> & Clue<LENGTH>::get_word(void) const noexcept
    {
        return this->_word;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    constexpr std::bitset<ALPHABET_NUM * LENGTH> State<LENGTH>::_get_word_mask(uint8_t word_pos)
    {
        static_assert(ALPHABET_NUM <= 64u);
        auto ret = std::bitset<ALPHABET_NUM * LENGTH>((1ull << ALPHABET_NUM) - 1);
        ret <<= ALPHABET_NUM * word_pos;
        return ret;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    State<LENGTH>::State(void) noexcept
    {
        this->_possible.set();
        this->_min_count.fill(0);
        this->_max_count.fill(LENGTH);
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    State<LENGTH>::State(const Clue<LENGTH> &clue) noexcept : State()
    {
        // Green
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (clue[i] == CLUE_GREEN)
            {
                auto cur_letter = clue.get_word()[i];
                this->_possible &= ~_get_word_mask(i);
                this->_possible.set(ALPHABET_NUM * i + cur_letter);
                ++this->_min_count[cur_letter];
                // Other letters
                ++this->_max_count[cur_letter];
                for (auto i = 0u; i < ALPHABET_NUM; ++i)
                {
                    if (this->_max_count[i] > this->_min_count[i])
                        --this->_max_count[i];
                }
            }
        }

        // Yellow & gray
        for (auto i = 0u; i < LENGTH; ++i)
        {
            auto cur_letter = clue.get_word()[i];
            switch (clue[i]) {
                case ClueType::CLUE_YELLOW:
                    this->_possible.reset(ALPHABET_NUM * i + cur_letter);
                    ++this->_min_count[cur_letter];
                    // Other letters
                    ++this->_max_count[cur_letter];
                    for (auto i = 0u; i < ALPHABET_NUM; ++i)
                    {
                        if (this->_max_count[i] > this->_min_count[i])
                            --this->_max_count[i];
                    }
                    break;
                case ClueType::CLUE_GRAY:
                    this->_possible.reset(ALPHABET_NUM * i + cur_letter);
                    this->_max_count[cur_letter] = this->_min_count[cur_letter];
                    break;
                default:
                    break;
            }
        }
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    State<LENGTH>::State(const Word<LENGTH> &guess, const Word<LENGTH> &answer) noexcept
    : State()
    {
        for (auto i = 0u; i < LENGTH; ++i)
        {
            auto cur_letter = i * ALPHABET_NUM + guess[i];
            if (guess[i] == answer[i])
            {
                this->_possible &= ~_get_word_mask(i);
                this->_possible.set(cur_letter);
            }
            else
            {
                this->_possible.reset(cur_letter);
            }
        }
        uint8_t sum_bincount = 0u;
        for (auto i = 0u; i < ALPHABET_NUM; ++i)
        {
            if (guess.bincount()[i] > 0)
            {
                if (guess.bincount()[i] <= answer.bincount()[i])
                {
                    this->_min_count[i] = guess.bincount()[i];
                }
                else
                {
                    this->_max_count[i] = this->_min_count[i] = answer.bincount()[i];
                }
                sum_bincount += this->_min_count[i];
            }
        }
        for (auto i = 0u; i < ALPHABET_NUM; ++i)
        {
            this->_max_count[i] = std::min(static_cast<uint8_t>(LENGTH + this->_min_count[i] - sum_bincount), this->_max_count[i]);
        }
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    bool State<LENGTH>::check(const Word<LENGTH> &word) const noexcept
    {
        // 1. Possibility
        for (auto i = 0u; i < LENGTH; ++i)
        {
            if (!this->_possible[ALPHABET_NUM * i + word[i]])
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

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    inline State<LENGTH> State<LENGTH>::operator&(const State<LENGTH> &other) const noexcept
    {
        State<LENGTH> ret{*this};
        ret &= other;
        return ret;
    }

    template <size_t LENGTH>
    requires ValidLength<LENGTH>
    State<LENGTH> &State<LENGTH>::operator&=(const State<LENGTH> &other) noexcept
    {
        this->_possible &= other._possible;
        for (auto i = 0u; i < ALPHABET_NUM; ++i)
        {
            this->_min_count[i] = std::max(this->_min_count[i], other._min_count[i]);
        }
        for (auto i = 0u; i < ALPHABET_NUM; ++i)
        {
            this->_max_count[i] = std::min(this->_max_count[i], other._max_count[i]);
        }

        uint8_t sum = 0;
        for (const auto &it : this->_min_count)
        {
            sum += it;
        }
        for (auto i = 0u; i < ALPHABET_NUM; ++i)
        {
            this->_max_count[i] = std::min(static_cast<uint8_t>(LENGTH + this->_min_count[i] - sum), this->_max_count[i]);
        }
        return *this;
    }

} // namespace wordle

template <size_t LENGTH>
requires wordle::ValidLength<LENGTH>
struct std::hash<wordle::Word<LENGTH>>
{
    size_t operator()(const wordle::Word<LENGTH> &object) const
    {
        size_t ret = 0u;
        size_t coefficient = 1u;
        for (auto i = 0u; i < LENGTH; ++i)
        {
            ret += coefficient * object[i];
            coefficient *= wordle::ALPHABET_NUM;
        }
        return ret;
    }

};

template <size_t LENGTH>
requires wordle::ValidLength<LENGTH>
struct std::hash<wordle::Clue<LENGTH>>
{
    size_t operator()(const wordle::Clue<LENGTH> &object) const
    {
        size_t ret = 0u;
        size_t coefficient = 1u;
        for (auto i = 0u; i < LENGTH; ++i)
        {
            ret += coefficient * (static_cast<size_t>(object[i]) + 1) * object.get_word()[i];
            coefficient *= wordle::ALPHABET_NUM * 3;
        }
        return ret;
    }
};

template <size_t LENGTH>
requires wordle::ValidLength<LENGTH>
struct std::hash<wordle::State<LENGTH>>
{
    size_t operator()(const wordle::State<LENGTH> &object) const
    {
        size_t seed = std::hash<std::bitset<LENGTH * wordle::ALPHABET_NUM>>()(object.get_possible_matrix());
        // boost::hash_combine(seed, object.get_possible_matrix());
        boost::hash_combine(seed, object.get_min_count());
        boost::hash_combine(seed, object.get_max_count());
        return seed;
    }
};

#endif /* WORDLE_MODEL_HPP */

