#include "OptionsWindow.h"
//#include <FractalGenerator/FractalParams.h>

OptionsWindow::OptionsWindow() :
	m_wnd_flags(0)
{
	m_wnd_flags |= ImGuiWindowFlags_NoCollapse;

	strcpy_s(m_formula_text.data(), example_code.size() + 1, example_code.c_str());
}

FractalParams OptionsWindow::Render(FractalParams prev, const std::string& device_name, bool params_changed)
{
	static double x_zoom = prev.zoom_x;
	static double y_zoom = prev.zoom_y;
	static bool use_gpu = prev.use_gpu;
	static bool use_custom_code = false;
	static bool equalize_zooms = true;
	static std::vector<std::pair<int, std::array<float, 3>>> percent_colours;

	if (percent_colours.size() != prev.colours.size())
	{
		percent_colours.clear();
		for (const auto& it : prev.colours)
			percent_colours.push_back({it.first * 100, it.second.GetFltArr()});
	}

	if (equalize_zooms)
		y_zoom = x_zoom;
	if (params_changed)
	{
		x_zoom = prev.zoom_x;
		y_zoom = prev.zoom_y;
		use_gpu = prev.use_gpu;
	}

	ImGui::Begin("Fractal Settings", nullptr, m_wnd_flags);
	ImGui::Checkbox("Use user defined algorithm", &use_custom_code);

	if (use_custom_code)
	{
		static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;


		//ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
		//ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", &flags, ImGuiInputTextFlags_AllowTabInput);
		//ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", &flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
		ImGui::InputTextMultiline("Custom Code", m_formula_text.data(), m_formula_text.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * max_code_lines), flags);

		if (ImGui::Button("Use"))
		{
			prev.use_custom_func = true;
			prev.custom_func_code = std::string(m_formula_text.data());
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			strcpy_s(m_formula_text.data(), example_code.size() + 1, example_code.c_str());
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(instuctions);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::InputDouble("Input zoom x", &x_zoom, 10.f, 100.0f, "%.0f");
	if (!equalize_zooms)
		ImGui::InputDouble("Input zoom y", &y_zoom, 10.f, 100.0f, "%.0f");
	ImGui::Checkbox("Equalize zooms", &equalize_zooms);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox("Use GPU", &use_gpu);
	if (use_gpu)
	{
		ImGui::Dummy({ 20, 10 });
		ImGui::SameLine();
		ImGui::Text(std::string("Running on: " + device_name).c_str());
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Colouring options:");

	int cnt = 0;
	std::vector<int> cols_to_remove;
	for (auto& [percent, col] : percent_colours)
	{
		const auto label = "Colour #" + std::to_string(++cnt) + " up to";
		const auto label_col_edit = "Colour #" + std::to_string(cnt);

		ImGui::DragInt(label.c_str(), &percent, 1, 0, 100, " % d % %", ImGuiSliderFlags_AlwaysClamp);
		ImGui::ColorEdit3(label_col_edit.c_str(), col.data());
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	static bool apply_on_fly = false;
	if (ImGui::Button("Apply") || apply_on_fly || ImGui::IsKeyDown(525))
	{
		prev.zoom_x = x_zoom;
		prev.zoom_y = equalize_zooms ? x_zoom : y_zoom;
		prev.colours.clear();

		for (auto& [percent, col] : percent_colours)
		{
			Colour empty;
			empty.SetFromFltArr(col);
			prev.colours.push_back({ percent / 100.f, empty });
		}
		prev.use_gpu = use_gpu;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Apply on fly", &apply_on_fly);

	ImGui::End();
	return prev;
}
