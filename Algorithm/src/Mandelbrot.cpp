#include "pch.h"

#include "Mandelbrot.hpp"

#include <complex>

using namespace std;

int Mandelbrot::getIter(double x0, double y0)
{
	int iter = 0;
	auto x = 0.0;
	auto y = 0.0;

	while (x * x + y * y <= 2 * 2 && iter < MAX_ITER)
	{
		auto xtemp = x * x - y * y + x0;
		y = 2 * x * y + y0;
		x = xtemp;
		iter++;
	}
	
	return iter;
}

boost::compute::function<int(DataCoord)> Mandelbrot::GetProcFuncGPU()
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
		int MandelbrotProcFunc(DataCoord c0)
		{
			int iter = 0;
			double x = 0.0;
			double y = 0.0;

			while (x * x + y * y <= 2 * 2 && iter < )" << MAX_ITER << R"()
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
				"MandelbrotProcFunc",
				src
				);
		ready = true;
	}
	
	return func;
}
