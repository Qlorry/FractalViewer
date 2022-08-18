#include "pch.h"
#include "Image.h"

template <class T>
Image<T>::Image(size_t width, size_t heigth)
	: m_heigth(heigth), m_width(width)
{
	m_data = new T*[m_heigth];
	for (auto i = 0; i < m_heigth; i++)
		m_data[i] = new T[m_width];
}

template <class T>
Image<T>::Image(Image&& other):
	m_width(other.m_width), m_heigth(other.m_heigth)
{
	m_data = other.m_data;

	other.m_width = 0;
	other.m_heigth = 0;
	other.m_data = nullptr;
}

template <class T>
Image<T>::~Image()
{
	Reset();
}

template <class T>
void Image<T>::Reset()
{
	for (auto i = 0; i < m_heigth; i++)
		delete[] m_data[i];
	delete[] m_data;

	m_width = 0;
	m_heigth = 0;
	m_data = nullptr;
}

template <class T>
void Image<T>::Resize(size_t width, size_t heigth)
{
	if (m_data)
		Reset();

	m_width = width;
	m_heigth = heigth;
	m_data = new T * [m_heigth];
	for (auto i = 0; i < m_heigth; i++)
		m_data[i] = new T[m_width];
	return;
}

template<class T>
Image<T>& Image<T>::operator=(Image<T>&& other)
{
	m_data = other.m_data;
	m_width = other.m_width;
	m_heigth = other.m_heigth;

	other.m_width = 0;
	other.m_heigth = 0;
	other.m_data = nullptr;
	return *this;
}

template class Image<Colour>;
template class Image<unsigned int>;