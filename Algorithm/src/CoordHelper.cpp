#include "pch.h"
#include "CoordHelper.h"

boost::compute::function<DataCoord(ImageCoord)> CoordHelper::GetImg2DataGPU(const FractalParams& p)
{
	std::ostringstream oss;
	oss << std::setprecision(12) << R"(
		DataCoord Img2Data(const ImageCoord img)
		{
			DataCoord res;
			double x0 = (double)img.x - )" << (static_cast<double>(p.width) / 2.) << R"(;
			double y0 = (double)img.y - )" << (static_cast<double>(p.heigth) / 2.) << R"(;

			res.x = x0 / )" << p.zoom_x << R"(;
			res.y = y0 / )" << p.zoom_y << R"(;

			res.x += )" << p.x << R"(;
			res.y += )" << p.y << R"(;

			return res;
		}
		)";

	const std::string src = oss.str();
	
	return boost::compute::make_function_from_source<DataCoord(ImageCoord c)>(
		"Img2Data",
		src
		);
}

DataCoord CoordHelper::Img2Data(const ImageCoord& img, const FractalParams& p)
{
	DataCoord res;
	auto x0 = static_cast<double>(img.x) - (p.width / 2.) ;
	auto y0 = static_cast<double>(img.y) - (p.heigth / 2.);

	res.x = x0 / p.zoom_x;
	res.y = y0 / p.zoom_y;

	res.x += p.x;
	res.y += p.y;

	return res;
}

ImageCoord CoordHelper::Data2Img(const DataCoord& data, const FractalParams& p)
{
	ImageCoord res;
	auto x0 = p.x - static_cast<long>(p.width / 2);
	auto y0 = p.y - static_cast<long>(p.heigth / 2);

	res.x = static_cast<long>(data.x * p.zoom_x) - x0;
	res.y = static_cast<long>(data.y * p.zoom_y) - y0;

	return res;
}
