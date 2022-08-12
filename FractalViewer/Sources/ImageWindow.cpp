#include "ImageWindow.h"

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
}

ImageWindow::ImageWindow() :
	m_wnd_flags(0)
{
	m_wnd_flags |= ImGuiWindowFlags_NoCollapse;
	m_wnd_flags |= ImGuiWindowFlags_NoScrollbar;
	m_wnd_flags |= ImGuiWindowFlags_NoNav;

	m_frac_params.x = -400;

	m_frac_params.colours.emplace_back(0.0f, Colour(0, 0, 255));
	m_frac_params.colours.emplace_back(0.05f, Colour(255, 150, 0));
	m_frac_params.colours.emplace_back(0.08f, Colour(255, 150, 0));
	m_frac_params.colours.emplace_back(1.f,  Colour(255, 255, 89));

	m_frac_params.zoom_x = 450.;
	m_frac_params.zoom_y = 450.;
	m_frac_params.use_gpu = true;
}

bool ImageWindow::LoadTexture(int image_width, int image_height)
{
	auto size = image_width * image_height * 4;
	unsigned char* image_data = new unsigned char[size];

	auto it = m_img.begin();
	for (auto i = 0; i < size; i += 4, it++)
	{
		auto r = image_data + i;
		auto g = image_data + i + 1;
		auto b = image_data + i + 2;
		auto a = image_data + i + 3;

		*r = it->r;
		*g = it->g;
		*b = it->b;
		*a = 255;
	}
	if (image_data == NULL)
		return false;

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
	return ImGui::GetWindowContentRegionMax();
}

void ImageWindow::Render(FractalParams p, bool& changed_params)
{
	ImGui::Begin("Fractal Display", nullptr, m_wnd_flags);                // Create a window called "Hello, world!" and append into it.
	ImVec2 image_size = GetImageSize();
	ImGuiStyle& style = ImGui::GetStyle();

	auto io = ImGui::GetIO();

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
	if (std::abs(io.MouseWheel) > std::numeric_limits<float>::epsilon())
	{
		params_changed = true;
		auto new_zoom_x = m_frac_params.zoom_x + io.MouseWheel * 15;
		auto new_zoom_y = m_frac_params.zoom_y + io.MouseWheel * 15;

		auto x_scale = new_zoom_x / m_frac_params.zoom_x;
		auto y_scale = new_zoom_y / m_frac_params.zoom_y;

		m_frac_params.x *= x_scale;
		m_frac_params.y *= y_scale;
		m_frac_params.zoom_x = new_zoom_x;
		m_frac_params.zoom_y = new_zoom_y;
	}
	if (ImGui::IsKeyDown(down_arr) || ImGui::IsKeyDown(s_key))
	{
		params_changed = true;
		m_frac_params.y += ImGui::IsKeyDown(shift_l) || ImGui::IsKeyDown(shift_r) ? 100 : 10;
	}
	if (ImGui::IsKeyDown(up_arr) || ImGui::IsKeyDown(w_key))
	{
		params_changed = true;
		m_frac_params.y -= ImGui::IsKeyDown(shift_l) || ImGui::IsKeyDown(shift_r) ? 100 : 10;;
	}
	if (ImGui::IsKeyDown(left_arr) || ImGui::IsKeyDown(a_key))
	{
		params_changed = true;
		m_frac_params.x -= ImGui::IsKeyDown(shift_l) || ImGui::IsKeyDown(shift_r) ? 100 : 10;;
	}
	if (ImGui::IsKeyDown(right_arr) || ImGui::IsKeyDown(d_key))
	{
		params_changed = true;
		m_frac_params.x += ImGui::IsKeyDown(shift_l) || ImGui::IsKeyDown(shift_r) ? 100 : 10;;
	}
	if (!m_img.IsValid() || m_frac_params.heigth != image_size.y || m_frac_params.width != image_size.x || params_changed)
	{
		m_frac_params.heigth = image_size.y;
		m_frac_params.width = image_size.x;
		m_img = std::move(m_generator.GenerateImage(m_frac_params));
		LoadTexture(image_size.x, image_size.y);
	}
	changed_params = params_changed;

	ImGui::Image((void*)(intptr_t)m_image_texture, image_size);
	ImGui::End();
}
