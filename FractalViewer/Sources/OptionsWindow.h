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
	std::array<char, 200> m_formula_text;
};

