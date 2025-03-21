//
// Created by Hash Liu on 2025/3/17.
//

#pragma once

#include <mutex>
#include <queue>

template <typename T>
class Queue
{
public:
    Queue() = default;
    void enqueue(T value);
    T dequeue();
    size_t size() const;
    bool empty() const;
    T front() const;
    T back() const;
private:
    std::queue<T>   m_queue;
    std::mutex      m_mutex;
};

template <typename T>
void Queue<T>::enqueue(T value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(value);
}

template <typename T>
T Queue<T>::dequeue()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    T value = m_queue.front();
    m_queue.pop();
    return value;
}

template <typename T>
size_t Queue<T>::size() const
{
    return m_queue.size();
}

template <typename T>
bool Queue<T>::empty() const
{
    return m_queue.empty();
}

template <typename T>
T Queue<T>::front() const
{
    return m_queue.front();
}

template <typename T>
T Queue<T>::back() const
{
    return m_queue.back();
}
