#pragma once

#include "Colour.h"
#include <iterator>

template <class T>
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;  // or also value_type*
    using reference = T&;  // or also value_type&

    Iterator(value_type** ptr, size_t max_shift, size_t shift) :
        m_ptr(ptr), m_current_shift(shift), m_max_shift(max_shift) {}

    reference operator*() const { return ((*m_ptr)[m_current_shift]); }
    pointer operator->() { return ((*m_ptr)+m_current_shift); }

    // Prefix increment
    virtual Iterator& operator++() 
    {
        m_current_shift++; 
        if (m_current_shift == m_max_shift)
        {
            m_ptr++;
            m_current_shift = 0;
        }
        return *this; 
    }

    // Postfix increment
    Iterator operator++(int) 
    { 
        Iterator tmp = *this; 
        m_current_shift++;
        if (m_current_shift == m_max_shift)
        {
            m_ptr++;
            m_current_shift = 0;
        }
        return tmp;
    }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.GetCurrent() == b.GetCurrent(); };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.GetCurrent() != b.GetCurrent(); };

    friend size_t operator- (const Iterator& a, const Iterator& b)
    {
        return a.m_max_shift * (a.m_ptr - b.m_ptr) + a.m_current_shift - b.m_current_shift; 
    }
protected:
    pointer GetCurrent() const { return (*m_ptr) + m_current_shift; }
    value_type** m_ptr;
    size_t m_current_shift;
    size_t m_max_shift;
};

template <class T>
class Image
{
public:
    Image() = default;
    Image(size_t width, size_t heigth);
    Image(Image& other) = delete;
    Image(Image&& other);
    ~Image();

    void Reset();
    void Resize(size_t width, size_t heigth);
    size_t GetWidth() const { return m_width; }
    size_t GetHeigth() const { return m_heigth; }

    Image<T> operator=(Image<T>&) = delete;
    Image<T>& operator=(Image<T>&& other);

    Iterator<T> at(size_t row, size_t column) { return Iterator(m_data + row, m_width, column); }

    Iterator<T> begin() { return Iterator(m_data, m_width, 0); }
    Iterator<T> end() { return Iterator(&m_data[m_heigth], m_width, 0); }

    bool IsValid() { return m_data && m_heigth > 0 && m_width > 0; }
private:
    T** m_data{ nullptr };

    size_t m_heigth{ 0u };
    size_t m_width{ 0u };
};

typedef Image<Colour> ColourImage;
typedef Image<unsigned int> DataImage;