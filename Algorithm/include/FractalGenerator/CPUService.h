#pragma once
#include "CoordHelper.h"
#include "Image.h"
#include "Mandelbrot.h"

class CPUService
{
public:
	CPUService();
	ColourImage GenerateImage(const FractalParams& p);
    std::string GetDevice() { return "CPU"; }
private:
    void CalculateHistogram();
    void CalculateData(const FractalParams& p);
    void CalculateImage(const FractalParams& p);

    Mandelbrot m_mandelbrot_alg;
    std::vector<Colour> m_palette;
    ColourImage m_image;
    DataImage m_data_img;
    std::vector<int> m_histogram;
};

