#include "pch.h"

#include "FractalGenerator.h"
#include "Mandelbrot.h"
#include "Histogram.h"

#include <iostream>
#include <chrono>

#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/filesystem.hpp>

FractalGenerator::FractalGenerator() 
{
}

ColourImage FractalGenerator::GenerateImage(FractalParams p)
{
	if (p.use_gpu && p.use_custom_func)
	{
		m_user_alg.UpdateFunctionCode(p.custom_func_code);
		if (!m_user_alg.IsFunctionValid())
			return { p.width, p.heigth };
		return std::move(m_gpu_generator.GenerateImage(p, &m_user_alg));
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

void FractalGenerator::ExportImage(ColourImage& img, const std::string& filename)
{
	boost::gil::rgb8_image_t export_img(img.GetWidth(), img.GetHeigth());
	auto v = boost::gil::view(export_img);
	auto img_it = img.begin();
	for (auto exp_img_it = v.begin(); exp_img_it != v.end(); exp_img_it++, img_it++)
	{
		*exp_img_it = boost::gil::rgb8_pixel_t{ img_it->r, img_it->g, img_it->b };
	}

	boost::filesystem::path dir("export_images");
	std::string path = std::string("export_images/") + filename;
	path += std::string(".png");
	if(boost::filesystem::is_directory(dir) || boost::filesystem::create_directory(dir))
		boost::gil::write_view(path, boost::gil::view(export_img), boost::gil::png_tag());
}
