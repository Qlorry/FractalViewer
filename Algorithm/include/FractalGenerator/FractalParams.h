#pragma once
#include "Colour.h"

#include <vector>
#include <string>

struct FractalParams
{
    size_t width;
    size_t heigth;

    double x{ 0. };
    double y{ 0. };

    double zoom_x{ 400. };
    double zoom_y{ 200. };

    std::vector<std::pair<float, Colour>> colours;

    bool use_gpu{ true };

    bool use_custom_func{ false };
    std::string custom_func_code;
};

