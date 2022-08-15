#include "pch.h"

#include "FractalGenerator.h"
#include "Mandelbrot.h"
#include "Histogram.h"

#include <iostream>
#include <chrono>


FractalGenerator::FractalGenerator() 
{
}

ColourImage FractalGenerator::GenerateImage(FractalParams p)
{
	if (p.use_gpu && p.use_custom_func)
	{
		Mandelbrot mandel_alg;
		return std::move(m_gpu_generator.GenerateImage(p, &mandel_alg));
	}
	else if (p.use_gpu)
	{
		Mandelbrot mandel_alg;
		return std::move(m_gpu_generator.GenerateImage(p, &mandel_alg));
	}
	else
	{
		return std::move(m_cpu_generator.GenerateImage(p));
	}
}
