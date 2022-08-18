#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "Colour.h"
#include "Image.h"
#include "FractalParams.h"
#include "GPUService.h"
#include "CPUService.h"
#include "UserDefinedAlgo.h"

class FractalGenerator {
public:

    FractalGenerator();
    ColourImage GenerateImage(FractalParams p);
    void ExportImage(ColourImage& img, std::string filename);

    std::string GetDevice() { return m_gpu_generator.GetDevice(); };
private:

    GPUService m_gpu_generator;
    CPUService m_cpu_generator;
    UserDefinedAlgo m_user_alg;
};