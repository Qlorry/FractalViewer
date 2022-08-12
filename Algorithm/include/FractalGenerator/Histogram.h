#pragma once
#include "Colour.hpp"
#include <vector>
class Histogram
{
public:

	Histogram(int min_value, int max_value, Colour min_col, Colour max_col) :
		m_min_value(min_value), m_max_value(max_value), m_min_col(min_col), m_max_col(max_col)
	{
		if (m_min_value > m_max_value)
			std::swap(m_min_value, m_max_value);
		
		m_palette.resize(m_max_value - m_min_value + 1);

		auto r_b_mod = GetBMod(m_min_col.r, m_max_col.r);
		auto g_b_mod = GetBMod(m_min_col.g, m_max_col.g);
		auto b_b_mod = GetBMod(m_min_col.b, m_max_col.b);
		for (int i = m_min_value; i < m_max_value + 1; i++)
		{
			m_palette[i].r = CalculateColourQuadratic(i, m_min_col.r, m_max_col.r, r_b_mod);
			m_palette[i].g = CalculateColourQuadratic(i, m_min_col.g, m_max_col.g, g_b_mod);
			m_palette[i].b = CalculateColourQuadratic(i, m_min_col.b, m_max_col.b, b_b_mod);
		}
	}

	Colour GetColour(int value)
	{
		return m_palette[value - m_min_value];
	}
private:
	unsigned char CalculateColourQuadratic(int value, int min, int max, float b_mod)
	{
		auto modifier = a_mod * value * value + b_mod * value;

		return min + modifier * value * (max - min);
	}

	unsigned char CalculateColourLinear(int value, int min, int max)
	{
		return min + (1.f / static_cast<float>(m_max_value - m_min_value)) * value * (max - min);
	}

	float GetBMod(int min, int max)
	{
		return ((0 - 1) + a_mod * m_max_value * m_max_value) / m_max_value;
	}

	std::vector<Colour> m_palette;

	int m_min_value;
	int m_max_value;
	Colour m_min_col;
	Colour m_max_col;
	float a_mod = 3;
};

