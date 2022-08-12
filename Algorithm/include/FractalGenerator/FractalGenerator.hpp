#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "Colour.hpp"
#include "Image.h"
#include "FractalParams.h"
#include "GPUService.h"

class FractalGenerator {
public:

    FractalGenerator();
    ColourImage GenerateImage(FractalParams p);

    std::string GetDevice() { return m_gpu_generator.GetDevice(); };
private:
    void CalculateHistogram();
    void CalculateData(const FractalParams& p);
    void CalculateImage(const FractalParams& p);

    std::vector<Colour> m_palette;
    ColourImage m_image;
    DataImage m_data_img;
    std::vector<int> m_histogram;
    GPUService m_gpu_generator;
};