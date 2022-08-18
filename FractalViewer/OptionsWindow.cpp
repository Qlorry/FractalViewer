#include "OptionsWindow.h"
#include <FractalGenerator/FractalGenerator.h>
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
	static double x_coord = prev.x;
	static double y_coord = prev.y;
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
		x_coord = prev.x;
		y_coord = prev.y;
		x_zoom = prev.zoom_x;
		y_zoom = prev.zoom_y;
		use_gpu = prev.use_gpu;
	}

	ImGui::Begin("Fractal Settings", nullptr, m_wnd_flags);

	if (ImGui::CollapsingHeader("Coordinates"))
	{
		ImGui::InputDouble("X coordinate", &x_coord, 10.f, 100.0f, "%.5f");
		ImGui::InputDouble("Y coordinate", &y_coord, 10.f, 100.0f, "%.5f");
	}

	if (ImGui::CollapsingHeader("Zoom"))
	{
		ImGui::InputDouble((std::string("Input zoom") + (equalize_zooms ? "" : " x")).c_str(), &x_zoom, 10.f, 100.0f, "%.0f");
		if (!equalize_zooms)
			ImGui::InputDouble("Input zoom y", &y_zoom, 10.f, 100.0f, "%.0f");
		ImGui::Checkbox("Equalize zooms", &equalize_zooms);
	}

	if (ImGui::CollapsingHeader("Colouring options"))
	{
		int cnt = 0;
		std::vector<int> cols_to_remove;
		for (auto& [percent, col] : percent_colours)
		{
			const auto label = "Colour #" + std::to_string(++cnt) + " up to";
			const auto label_col_edit = "Colour #" + std::to_string(cnt);

			ImGui::DragInt(label.c_str(), &percent, 1, 0, 100, " % d % %", ImGuiSliderFlags_AlwaysClamp);
			ImGui::ColorEdit3(label_col_edit.c_str(), col.data());
		}
	}

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

	static bool apply_on_fly = false;
	if (ImGui::Button("Apply") || apply_on_fly || ImGui::IsKeyDown(525))
	{
		prev.x = x_coord;
		prev.y = y_coord;
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

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Use user defined algorithm"))
	{
		static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
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
	if (ImGui::CollapsingHeader("Export image"))
	{
		static int w = prev.width * 4;
		static int h = prev.heigth * 4;
		w = prev.width * 4;
		h = prev.heigth * 4;
		static std::string name;
		name.resize(250);

		ImGui::InputInt("Horizontal size", &w);
		ImGui::InputInt("Vertical size", &h);
		ImGui::InputText("Filename", name.data(), 250);
		if (ImGui::Button("Run"))
		{
			if (!name.empty())
			{
				FractalGenerator serv;
				auto temp = prev;
				temp.width = w;
				temp.heigth = h;

				temp.zoom_x *= (w / prev.width);
				temp.zoom_y *= (h / prev.heigth);

				auto img = serv.GenerateImage(temp);
				serv.ExportImage(img, name);
			}
		}
	}

	ImGui::End();
	return prev;
}
