#include <ranges>
#include <type_traits>
#ifndef WORDLE_PRIORITY_QUEUE_HPP
#define WORDLE_PRIORITY_QUEUE_HPP 1

#include <vector>
#include <utility>
#include <algorithm>

namespace wordle
{
    template <typename T, typename Compare = std::less<T>>
    class priority_queue
    {
    public:
        std::vector<T>& data();
        const std::vector<T>& data() const;
        void make_heap();
        template <typename U>
        void push(U&& value);
        template <std::ranges::viewable_range R>
        void push(R&& range);
        template <typename... Args>
        void emplace(Args&&... args);
        void pop();
        const T& top() const;
        template <typename U>
        void pushpop(U&& value);
        bool is_empty() const;
    private:
        std::vector<T> _data;
    };

    template <typename T, typename Compare>
    std::vector<T>& priority_queue<T, Compare>::data()
    {
        return this->_data;
    }

    template <typename T, typename Compare>
    const std::vector<T>& priority_queue<T, Compare>::data() const
    {
        return this->_data;
    }

    template <typename T, typename Compare>
    void priority_queue<T, Compare>::make_heap()
    {
        std::ranges::make_heap(this->_data, Compare{});
    }

    template <typename T, typename Compare>
    template <typename U>
    void priority_queue<T, Compare>::push(U&& value)
    {
        static_assert(std::is_convertible_v<U, T>);
        this->_data.push_back(std::forward<U>(value));
        std::ranges::push_heap(this->_data, Compare{});
    }

    template <typename T, typename Compare>
    template <std::ranges::viewable_range R>
    void priority_queue<T, Compare>::push(R&& range)
    {
        this->_data.append_range(std::forward<R>(range));
        this->make_heap();
    }

    template <typename T, typename Compare>
    template <typename... Args>
    void priority_queue<T, Compare>::emplace(Args&&... args)
    {
        this->_data.emplace_back(std::forward<Args>(args)...); // TODO
        std::ranges::push_heap(this->_data, Compare{});
    }

    template <typename T, typename Compare>
    void priority_queue<T, Compare>::pop()
    {
        std::ranges::pop_heap(this->_data, Compare{});
        this->_data.pop_back();
    }

    template <typename T, typename Compare>
    const T& priority_queue<T, Compare>::top() const
    {
        return this->_data[0];
    }

    template <typename T, typename Compare>
    template <typename U>
    void priority_queue<T, Compare>::pushpop(U&& value)
    {
        static_assert(std::is_convertible_v<U, T>);
        Compare compare{};
        if (this->is_empty() || compare(this->_data[0], value))
        {
            return;
        }
        this->_data.push_back(std::forward<U>(value));
        std::ranges::pop_heap(this->_data, compare);
        this->_data.pop_back();
    }

    template <typename T, typename Compare>
    bool priority_queue<T, Compare>::is_empty() const
    {
        return this->_data.empty();
    }

} /* namespace wordle */ 

#endif /* ifndef WORDLE_PRIORITY_QUEUE_HPP */
