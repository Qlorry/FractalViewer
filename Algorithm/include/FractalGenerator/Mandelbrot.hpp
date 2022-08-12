#pragma once
#include "CoordHelper.h"
#include <boost/compute/types/struct.hpp>

#include <string>
#include <sstream>

class Mandelbrot {
public:
    static const int MAX_ITER = 1000;
        
public:
    Mandelbrot() = default;
    ~Mandelbrot() = default;
        
    static int getIter(double x, double y);
    static boost::compute::function<int (DataCoord)> GetProcFuncGPU();
};

