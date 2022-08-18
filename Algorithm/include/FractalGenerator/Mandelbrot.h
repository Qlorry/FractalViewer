#pragma once
#include "CoordHelper.h"
#include "FractalAlgo.h"


class Mandelbrot final : public FractalAlgo
{
public:
    Mandelbrot() = default;
    ~Mandelbrot() = default;
        
    unsigned int ProcessCoord(const DataCoord& coord) override;
    boost::compute::function<unsigned int(DataCoord)> GetProcessCoordGPU() override;
};

