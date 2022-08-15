#pragma once
#include "CoordHelper.h"
#include "FractalAlgo.h"


class Mandelbrot final : public FractalAlgo
{
public:
    Mandelbrot() = default;
    ~Mandelbrot() = default;
        
    int ProcessCoord(const DataCoord& coord) const override;
    boost::compute::function<int(DataCoord)> GetProcessCoordGPU() const override;
};

