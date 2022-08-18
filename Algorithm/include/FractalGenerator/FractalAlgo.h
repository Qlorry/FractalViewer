#pragma once
#include "boost/compute/function.hpp"

struct DataCoord;

class FractalAlgo
{
public:
    FractalAlgo() = default;
    virtual ~FractalAlgo() = default;

    virtual unsigned int ProcessCoord(const DataCoord& coord) = 0;
    virtual boost::compute::function<unsigned int(DataCoord)> GetProcessCoordGPU() = 0;
    virtual unsigned int GetMaxIterations() const { return max_iterations; }
protected:
    unsigned int max_iterations = 1000u;
};


