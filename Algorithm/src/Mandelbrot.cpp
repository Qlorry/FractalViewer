#include "pch.h"

#include "Mandelbrot.h"

#include <string>
#include <sstream>
#include <complex>

int Mandelbrot::ProcessCoord(const DataCoord& coord)
{
	int iter = 0;
	auto x = 0.0;
	auto y = 0.0;

	while (x * x + y * y <= 2 * 2 && iter < max_iterations)
	{
		auto xtemp = x * x - y * y + coord.x;
		y = 2 * x * y + coord.y;
		x = xtemp;
		iter++;
	}
	
	return iter;
}

boost::compute::function<int(DataCoord)> Mandelbrot::GetProcessCoordGPU() 
{
	static bool ready = false;
	static auto func = boost::compute::make_function_from_source<int(DataCoord c)>(
		"MandelbrotProcFunc", 
		""
		);

	if (!ready)
	{
		std::ostringstream oss;
		oss << R"(
		int MandelbrotDataCoordFunc(DataCoord c0)
		{
			int iter = 0;
			double x = 0.0;
			double y = 0.0;

			while (x * x + y * y <= 2 * 2 && iter < )" << max_iterations << R"()
			{
				double xtemp = x * x - y * y + c0.x;
				y = 2 * x * y + c0.y;
				x = xtemp;
				iter++;
			}

			return iter;
		}
		)";

		const std::string src = oss.str();
		func = boost::compute::make_function_from_source<int(DataCoord c)>(
				"MandelbrotDataCoordFunc",
				src
				);
		ready = true;
	}
	
	return func;
}
