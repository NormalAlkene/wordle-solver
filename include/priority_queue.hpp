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
        void push(T&& value);
        template <typename... Args>
        void emplace(Args&&... args);
        void pop();
        const T& top() const;
        void pushpop(T&& value);
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
    void priority_queue<T, Compare>::push(T&& value)
    {
        this->_data.push_back(std::forward<T>(value));
        std::ranges::push_heap(this->_data, Compare{});
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
    void priority_queue<T, Compare>::pushpop(T&& value)
    {
        Compare compare{};
        if (compare(this->_data[0], value))
        {
            return;
        }
        this->_data.push_back(std::forward<T>(value));
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
