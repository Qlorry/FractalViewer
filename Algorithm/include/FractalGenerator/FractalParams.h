#pragma once
#include "Colour.hpp"

#include <vector>
struct FractalParams
{
    size_t width;
    size_t heigth;

    long x{ 0 };
    long y{ 0 };

    double zoom_x{ 400. };
    double zoom_y{ 200. };

    std::vector<std::pair<float, Colour>> colours;

    bool use_gpu{ true };
};

