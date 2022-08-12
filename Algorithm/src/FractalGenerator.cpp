#include "pch.h"

#include "FractalGenerator.hpp"
#include "CoordHelper.h"
#include "Mandelbrot.hpp"
#include "Histogram.h"
#include "Colour.hpp"

#include <iostream>
#include <chrono>


FractalGenerator::FractalGenerator() 
{
}

ColourImage FractalGenerator::GenerateImage(FractalParams p)
{
	using namespace std::chrono;
	if (!p.use_gpu)
	{
		m_image = std::move(ColourImage(p.width, p.heigth));
		m_data_img.Resize(p.width, p.heigth);
	}

	auto start = std::chrono::high_resolution_clock::now();
	auto end = start;
	std::cout << "#1. Started Generating Image\n";
	if (p.use_gpu)
	{
		m_image = std::move(m_gpu_generator.GenerateImage(p));
		end = std::chrono::high_resolution_clock::now();
		std::cout << "#1. finished: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
	}
	else
	{
		CalculateData(p);
		auto step_2 = std::chrono::high_resolution_clock::now();
		std::cout << "#1. finished: " << duration_cast<milliseconds>(step_2 - start).count() << " ms\n";

		std::cout << "#2. Calculating Histogram\n";
		//if (!p.use_gpu)
		CalculateHistogram();
		auto step_3 = std::chrono::high_resolution_clock::now();
		std::cout << "#2. finished: " << duration_cast<milliseconds>(step_3 - step_2).count() << " ms\n";

		std::cout << "#3. Started Colouring\n";
		//if (p.use_gpu)
		//	m_image = std::move(m_gpu_generator.GenerateImage(p));
		//else
		CalculateImage(p);
		end = std::chrono::high_resolution_clock::now();
		std::cout << "#3. finished: " << duration_cast<milliseconds>(end - step_3).count() << " ms\n";
	}
	
	std::cout << "Time to for all " << duration_cast<milliseconds>(end - start).count() << " ms\n" << std::endl;

	return std::move(m_image);
}

void FractalGenerator::CalculateHistogram()
{
	m_histogram.clear();
	m_histogram.resize(Mandelbrot::MAX_ITER);
	for (auto data_it = m_data_img.begin(); data_it != m_data_img.end(); data_it++)
	{
		if (*data_it != Mandelbrot::MAX_ITER)
			m_histogram[*data_it]++;
	}
}

void FractalGenerator::CalculateData(const FractalParams& p)
{
	for (unsigned int y = 0u; y < p.heigth; y++) {
		for (unsigned int x = 0u; x < p.width; x++) {
			auto [data_x, data_y] = CoordHelper::Img2Data({ x, y }, p);

			*m_data_img.at(y, x) = Mandelbrot::getIter(data_x, data_y);
		}
	}
}

void FractalGenerator::CalculateImage(const FractalParams& p)
{
	std::vector<float> range;
	std::vector<int> numOfRanges;
	for (auto& c : p.colours)
	{
		range.push_back(c.first * Mandelbrot::MAX_ITER);
		numOfRanges.push_back(0);
	}

	int rangeIndex = 0;
	for (int i = 0; i < Mandelbrot::MAX_ITER; i++)
	{
		int pixels = m_histogram[i];

		if (i >= range[rangeIndex + 1]) { rangeIndex++; }

		numOfRanges[rangeIndex] += pixels;
	}

	const auto GetRange = [&](int value)
	{
		int ran = 0;

		for (int i = 1; i < range.size(); i++)
		{
			if (range[i] > value) { break; }
			ran = i;
		}
		return ran;
	};

	m_palette.resize(Mandelbrot::MAX_ITER+1);
	for (int it = 0; it <= Mandelbrot::MAX_ITER; it++)
	{
		Colour result(37, 37, 37);
	
		if (it != Mandelbrot::MAX_ITER)
		{
			auto ran = GetRange(it);
			int rangeTotal = numOfRanges[ran];
			int rangeStart = range[ran];

			auto startColor = p.colours[ran].second;
			auto endColor = ran + 1 >= p.colours.size() ? Colour(0, 0, 0) : p.colours[ran + 1].second;
			Colour colorDiff = endColor - startColor;

			int totalPixels = 0;
			for (int i = rangeStart; i <= it; i++) { totalPixels += m_histogram[i]; }

			result.r = startColor.r + ((colorDiff.r * (double)totalPixels) / rangeTotal);
			result.g = startColor.g + ((colorDiff.g * (double)totalPixels) / rangeTotal);
			result.b = startColor.b + ((colorDiff.b * (double)totalPixels) / rangeTotal);
		}
		m_palette[it] = result;
	}

	for (unsigned int y = 0u; y < p.heigth; y++) 
	{
		for (unsigned int x = 0u; x < p.width; x++) 
		{
			int it = *m_data_img.at(y, x);
			*m_image.at(y, x) = m_palette[it];
		}
	}
}