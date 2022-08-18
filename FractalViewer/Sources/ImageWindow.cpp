#include "ImageWindow.h"
#include <future>

namespace
{
	const ImGuiKey down_arr = 516;
	const ImGuiKey s_key = 564;

	const ImGuiKey up_arr = 515;
	const ImGuiKey w_key = 568;

	const ImGuiKey left_arr = 513;
	const ImGuiKey a_key = 546;

	const ImGuiKey right_arr = 514;
	const ImGuiKey d_key = 549;

	const ImGuiKey shift_l = 528;
	const ImGuiKey shift_r = 532;

	const ImGuiKey ctrl_l = 527;
	const ImGuiKey ctrl_r = 531;
}

ImageWindow::ImageWindow() :
	m_wnd_flags(0)
{
	m_wnd_flags |= ImGuiWindowFlags_NoCollapse;
	m_wnd_flags |= ImGuiWindowFlags_NoScrollbar;
	m_wnd_flags |= ImGuiWindowFlags_NoNav;

	m_frac_params.x = -400.;

	m_frac_params.colours.emplace_back(0.0f, Colour(0, 0, 255));
	m_frac_params.colours.emplace_back(0.05f, Colour(255, 150, 0));
	m_frac_params.colours.emplace_back(0.9f, Colour(255, 240, 80));
	m_frac_params.colours.emplace_back(1.f,  Colour(255, 255, 89));

	m_frac_params.zoom_x = 450.;
	m_frac_params.zoom_y = 450.;
	m_frac_params.use_gpu = true;
}

bool ImageWindow::LoadTexture(int image_width, int image_height)
{
	auto size = image_width * image_height * 4;
	unsigned char* image_data = new unsigned char[size];

	if (image_data == NULL)
		return false;

	std::vector<std::future<void>> futures;
	futures.reserve(image_height);

	for (size_t y = 0u; y < image_height; y++)
	{
		futures.push_back(
			std::async([this, data = (image_data + (y* image_width*4)), y, width = image_width]() {
			for (size_t x = 0; x < width * 4; x+=4)
			{
				auto r = data + x;
				auto g = data + x + 1;
				auto b = data + x + 2;
				auto a = data + x + 3;
				auto it = m_img.at(y, x/4);
				*r = it->r;
				*g = it->g;
				*b = it->b;
				*a = 255;
			}
		})
		);
	}
	for (const auto& f : futures)
		f.wait();

	if(m_texture_valid)
		glDeleteTextures(1, &m_image_texture);
	// Create a OpenGL texture identifier
	glGenTextures(1, &m_image_texture);
	glBindTexture(GL_TEXTURE_2D, m_image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	delete[] image_data;

	return true;
}

ImVec2 ImageWindow::GetImageSize()
{
	return ImGui::GetContentRegionAvail();
}

void ImageWindow::Render(FractalParams p, bool& changed_params)
{
	ImGui::Begin("Fractal Display", nullptr, m_wnd_flags);
	ImVec2 image_size = GetImageSize();
	ImGuiStyle& style = ImGui::GetStyle();

	auto io = ImGui::GetIO();
	bool is_shift_pressed = ImGui::IsKeyDown(shift_l) || ImGui::IsKeyDown(shift_r);
	bool is_ctrl_pressed = ImGui::IsKeyDown(ctrl_l) || ImGui::IsKeyDown(ctrl_r);
	auto move_mod = 10.;
	if (is_shift_pressed)
		move_mod = 15.;
	if (is_ctrl_pressed)
		move_mod = 1.;
	bool active = ImGui::IsWindowFocused();
	bool params_changed = false;
	if ((std::abs(m_frac_params.zoom_x - p.zoom_x) > std::numeric_limits<float>::epsilon() ||
		std::abs(m_frac_params.zoom_y - p.zoom_y) > std::numeric_limits<float>::epsilon()) &&
		p.zoom_x > 0 && p.zoom_y > 0)
	{
		params_changed = true;
		auto x_scale = p.zoom_x / m_frac_params.zoom_x;
		auto y_scale = p.zoom_y / m_frac_params.zoom_y;

		m_frac_params.x *= x_scale;
		m_frac_params.y *= y_scale;
		m_frac_params.zoom_x = p.zoom_x;
		m_frac_params.zoom_y = p.zoom_y;
	}
	else if (std::abs(m_frac_params.x - p.x) > std::numeric_limits<float>::epsilon() ||
		std::abs(m_frac_params.y - p.y) > std::numeric_limits<float>::epsilon())
	{
		params_changed = true;
		m_frac_params.x = p.x;
		m_frac_params.y = p.y;
	}
	if (m_frac_params.use_gpu != p.use_gpu)
	{
		params_changed = true;
		m_frac_params.use_gpu = p.use_gpu;
	}
	if (m_frac_params.colours != p.colours)
	{
		params_changed = true;
		m_frac_params.colours = p.colours;
	}

	if (active)
	{
		if (std::abs(io.MouseWheel) > std::numeric_limits<float>::epsilon())
		{
			params_changed = true;
			//auto one_percent_x = m_frac_params.zoom_x / 100;
			//auto one_percent_y = m_frac_params.zoom_y / 100;
			//auto new_zoom_x = m_frac_params.zoom_x + one_percent_x * 10. * std::round(io.MouseWheel);
			//auto new_zoom_y = m_frac_params.zoom_y + one_percent_y * 10. * std::round(io.MouseWheel);

			auto x_scale = 1.1;//new_zoom_x / m_frac_params.zoom_x;
			auto y_scale = 1.1;//new_zoom_y / m_frac_params.zoom_y;

			if (io.MouseWheel < 0)
			{
				m_frac_params.zoom_x /= x_scale;
				m_frac_params.zoom_y /= y_scale;
				m_frac_params.x /= x_scale;
				m_frac_params.y /= y_scale;
			}
			else
			{
				m_frac_params.zoom_x *= x_scale;
				m_frac_params.zoom_y *= y_scale;
				m_frac_params.x *= x_scale;
				m_frac_params.y *= y_scale;
			}
		}
		if (ImGui::IsKeyDown(down_arr) || ImGui::IsKeyDown(s_key))
		{
			params_changed = true;
			auto one_pr = image_size.y / 100.;
			m_frac_params.y += one_pr * move_mod;
		}
		if (ImGui::IsKeyDown(up_arr) || ImGui::IsKeyDown(w_key))
		{
			params_changed = true;
			auto one_pr = image_size.y / 100.;
			m_frac_params.y -= one_pr * move_mod;
		}
		if (ImGui::IsKeyDown(left_arr) || ImGui::IsKeyDown(a_key))
		{
			params_changed = true;
			auto one_pr = image_size.x / 100.;
			m_frac_params.x -= one_pr * move_mod;
		}
		if (ImGui::IsKeyDown(right_arr) || ImGui::IsKeyDown(d_key))
		{
			params_changed = true;
			auto one_pr = image_size.x / 100.;
			m_frac_params.x += one_pr * move_mod;
		}
	}
	if (m_frac_params.use_custom_func != p.use_custom_func || (p.use_custom_func && m_frac_params.custom_func_code != p.custom_func_code))
	{
		params_changed = true;
		m_frac_params.use_custom_func = p.use_custom_func;
		m_frac_params.custom_func_code = p.custom_func_code;
	}
	if (!m_img.IsValid() || m_frac_params.heigth != image_size.y || m_frac_params.width != image_size.x || params_changed)
	{
		m_frac_params.heigth = static_cast<size_t>(image_size.y);
		m_frac_params.width = static_cast<size_t>(image_size.x);
		m_img = std::move(m_generator.GenerateImage(m_frac_params));
		LoadTexture(static_cast<int>(image_size.x), static_cast<int>(image_size.y));
	}
	changed_params = params_changed;

	ImGui::Image((void*)(intptr_t)m_image_texture, image_size);
	ImGui::End();
}
