#include "pch.h"
#include "UserDefinedAlgo.h"

#include "boost/compute/program.hpp"
#include "boost/compute/kernel.hpp"
#include <boost/compute/system.hpp>

UserDefinedAlgo::UserDefinedAlgo(std::string code) : m_code(std::move(code))
{
	ValidateFunction();
}

int UserDefinedAlgo::ProcessCoord(const DataCoord& coord)
{
	return 0;
}

boost::compute::function<int(DataCoord)> UserDefinedAlgo::GetProcessCoordGPU()
{
	static auto func = boost::compute::make_function_from_source<int(DataCoord c)>(
		"UserProcFunc",
		""
		);

	if (!m_is_function_ready)
	{
		std::ostringstream oss;
		oss << R"(
		int UserProcFunc(DataCoord coordinate)
		{
			int iteration = 0;
			double x = 0.0;
			double y = 0.0;

			while (x * x + y * y <= 2 * 2 && iteration < )" << max_iterations << R"()
			{
			)"
			<< m_code <<
			R"(
				iteration++;
			}

			return iteration;
		}
		)";

		const std::string src = oss.str();
		func = boost::compute::make_function_from_source<int(DataCoord c)>(
			"UserProcFunc",
			src
			);
		m_is_function_ready = true;
	}

	return func;
}

void UserDefinedAlgo::UpdateFunctionCode(const std::string& code)
{
	if (IsSameFunction(code))
		return;
	m_is_function_ready = false;
	m_code = code;
	m_is_valid = false;
	ValidateFunction();
}

void UserDefinedAlgo::ValidateFunction()
{
	try
	{
		auto func = GetProcessCoordGPU();
		auto source_string = boost::compute::type_definition<DataCoord>() + func.source();

		auto device = boost::compute::system::default_device();
		auto context = boost::compute::context(device);

		boost::compute::program p;
		p.build_with_source(source_string, context);
	}
	catch (boost::compute::opencl_error e)
	{
		m_is_valid = false;
	}
	m_is_valid = true;
}
