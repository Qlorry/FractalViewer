#pragma once
#include "FractalParams.h"

#include <boost/compute/types/struct.hpp>

#include <string>
#include <sstream>

struct DataCoord
{
	double x;
	double y;
};

struct ImageCoord
{
	size_t x;
	size_t y;
};

BOOST_COMPUTE_ADAPT_STRUCT(DataCoord, DataCoord, (x, y))
BOOST_COMPUTE_ADAPT_STRUCT(ImageCoord, ImageCoord, (x, y))

class CoordHelper
{
public:
	static boost::compute::function<DataCoord(ImageCoord)> GetImg2DataGPU(const FractalParams& p);

	static DataCoord Img2Data(const ImageCoord& img, const FractalParams& p);

	static ImageCoord Data2Img(const DataCoord& data, const FractalParams& p);
};

