#pragma once
#include "boost/compute/function.hpp"

struct DataCoord;

class FractalAlgo
{
public:
    FractalAlgo() = default;
    virtual ~FractalAlgo() = default;

    virtual int ProcessCoord(const DataCoord& coord) const = 0;
    virtual boost::compute::function<int(DataCoord)> GetProcessCoordGPU() const = 0;
    virtual int GetMaxIterations() const { return max_iterations; }
protected:
    int max_iterations = 1000;
};


