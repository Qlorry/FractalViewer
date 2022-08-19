#include "pch.h"
#include "CPUService.h"
#include <iostream>
#include <Mandelbrot.h>

CPUService::CPUService()
{
}

ColourImage CPUService::GenerateImage(const FractalParams& p)
{
	using namespace std::chrono;
	m_image = std::move(ColourImage(p.width, p.heigth));
	m_data_img.Resize(p.width, p.heigth);

	auto start = std::chrono::high_resolution_clock::now();
	auto end = start;
	std::cout << "#1. Started Generating Image\n";

	CalculateData(p);
	auto step_2 = std::chrono::high_resolution_clock::now();
	std::cout << "#1. finished: " << duration_cast<milliseconds>(step_2 - start).count() << " ms\n";

	std::cout << "#2. Calculating Histogram\n";
	CalculateHistogram();
	auto step_3 = std::chrono::high_resolution_clock::now();
	std::cout << "#2. finished: " << duration_cast<milliseconds>(step_3 - step_2).count() << " ms\n";

	std::cout << "#3. Started Colouring\n";

	CalculateImage(p);
	end = std::chrono::high_resolution_clock::now();
	std::cout << "#3. finished: " << duration_cast<milliseconds>(end - step_3).count() << " ms\n";

	std::cout << "Time to for all " << duration_cast<milliseconds>(end - start).count() << " ms\n" << std::endl;

	return std::move(m_image);
}


void CPUService::CalculateHistogram()
{
	m_histogram.clear();
	m_histogram.resize(m_mandelbrot_alg.GetMaxIterations());
	for (auto data_it = m_data_img.begin(); data_it != m_data_img.end(); data_it++)
	{
		if (*data_it != m_mandelbrot_alg.GetMaxIterations())
			m_histogram[*data_it]++;
	}
}

void CPUService::CalculateData(const FractalParams& p)
{
	for (size_t y = 0u; y < p.heigth; y++) {
		for (size_t x = 0u; x < p.width; x++) {
			auto coord = CoordHelper::Img2Data({ x, y }, p);
			*m_data_img.at(y, x) = m_mandelbrot_alg.ProcessCoord(coord);
		}
	}
}

void CPUService::CalculateImage(const FractalParams& p)
{
	std::vector<float> range;
	std::vector<size_t> numOfRanges;
	for (auto& c : p.colours)
	{
		range.push_back(c.first * m_mandelbrot_alg.GetMaxIterations());
		numOfRanges.push_back(0);
	}

	int rangeIndex = 0;
	for (unsigned int i = 0; i < m_mandelbrot_alg.GetMaxIterations(); i++)
	{
		size_t pixels = m_histogram[i];

		if (i >= range[rangeIndex + 1]) { rangeIndex++; }

		numOfRanges[rangeIndex] += pixels;
	}

	const auto GetRange = [&](unsigned int value)
	{
		unsigned int ran = 0;

		for (int i = 1; i < range.size(); i++)
		{
			if (range[i] > value) { break; }
			ran = i;
		}
		return ran;
	};

	m_palette.resize(m_mandelbrot_alg.GetMaxIterations() + 1);
	for (unsigned int it = 0; it <= m_mandelbrot_alg.GetMaxIterations(); it++)
	{
		Colour result(37, 37, 37);

		if (it != m_mandelbrot_alg.GetMaxIterations())
		{
			auto ran = GetRange(it);
			auto rangeTotal = numOfRanges[ran];
			float rangeStart = range[ran];

			auto startColor = p.colours[ran].second;
			auto endColor = ran + 1 >= p.colours.size() ? Colour(0, 0, 0) : p.colours[ran + 1].second;
			Colour colorDiff = endColor - startColor;

			size_t totalPixels = 0;
			for (unsigned int i = rangeStart; i <= it; i++) { totalPixels += m_histogram[i]; }

			result.r = startColor.r + ((colorDiff.r * (double)totalPixels) / rangeTotal);
			result.g = startColor.g + ((colorDiff.g * (double)totalPixels) / rangeTotal);
			result.b = startColor.b + ((colorDiff.b * (double)totalPixels) / rangeTotal);
		}
		m_palette[it] = result;
	}

	for (size_t y = 0u; y < p.heigth; y++)
	{
		for (size_t x = 0u; x < p.width; x++)
		{
			unsigned int it = *m_data_img.at(y, x);
			*m_image.at(y, x) = m_palette[it];
		}
	}
}