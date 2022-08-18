#pragma once
#include "CoordHelper.h"
#include "Image.h"
#include "FractalAlgo.h"

#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/math.hpp>

namespace compute = boost::compute;

class GPUService
{
public:
	GPUService();
	ColourImage GenerateImage(const FractalParams& p, FractalAlgo* alg);
	std::string GetDevice();
private:
	compute::vector<DataCoord> CalculateCoordImage(const FractalParams& p);
	std::pair<std::vector<unsigned int>, compute::vector<unsigned int>> CalculateDataImage(compute::vector<DataCoord>& input, const FractalParams& p);
	ColourImage CalculateColours(const FractalParams& p, const compute::vector<unsigned int>& data, const std::vector<size_t>& histogram);
	
	compute::vector<Colour> CalculatePalette(const FractalParams& p, const std::vector<size_t>& histogram_cpu);
	std::vector<Colour> CalculateImageColours(const FractalParams& p, const compute::vector<unsigned int>& data_gpu, const compute::vector<Colour>& palette_gpu);

	FractalAlgo* m_alg;

	compute::device m_device;
	compute::context m_context;
	compute::command_queue m_queue;
	compute::program_cache m_cache;
};

