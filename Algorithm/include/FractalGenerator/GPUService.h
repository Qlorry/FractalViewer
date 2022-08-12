#pragma once
#include "CoordHelper.h"

#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/math.hpp>

namespace compute = boost::compute;

class GPUService
{
public:
	GPUService();
	ColourImage GenerateImage(const FractalParams& p);
	std::string GetDevice();
private:
	compute::vector<DataCoord> CalculateCoordImage(const FractalParams& p);
	std::pair<std::vector<int>, compute::vector<int>> CalculateDataImage(compute::vector<DataCoord>& input, const FractalParams& p);
	ColourImage CalculateColours(const FractalParams& p, const compute::vector<int>& data, const std::vector<int>& histogram);
	
	compute::vector<boost::compute::uchar4_> CalculatePalette(const FractalParams& p, const std::vector<int>& histogram_cpu);
	std::vector<Colour> CalculateImageColours(const FractalParams& p, const compute::vector<int>& data_gpu, const compute::vector<boost::compute::uchar4_>& palette_gpu);

	compute::device m_device;
	compute::context m_context;
	compute::command_queue m_queue;
	compute::program_cache m_cache;
};

