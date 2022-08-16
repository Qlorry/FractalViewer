#pragma once
#include "imgui.h"

#include <FractalGenerator/FractalParams.h>

#include <string>
#include <array>

class OptionsWindow
{
public:
	OptionsWindow();

	FractalParams Render(FractalParams prev, const std::string& device_name, bool params_changed);
private:
	ImGuiWindowFlags m_wnd_flags;
	constexpr static int max_code_lines = 15;
	constexpr static auto instuctions = "This Field is for your custom fractal formula.\nThis code will be run in cycle all you need is to set 'x' and 'y' variables. Current coordinate(z0) is 'coordinate.x' and 'coordinate.y'\nUse C only, you may create any temporary variables if needed.";
	std::string example_code = R"(
double mod_x = x > 0 ? x : -x;
double mod_y = y > 0 ? y : -y;
double xtemp = mod_x * mod_x - mod_y * mod_y + coordinate.x;
y = 2 * mod_x * mod_y + coordinate.y; 
x = xtemp;
)";
	std::array<char, 1024 * max_code_lines> m_formula_text;
};

