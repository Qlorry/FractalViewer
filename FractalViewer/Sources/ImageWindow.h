#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "FractalGenerator/Image.h"
#include "FractalGenerator/FractalGenerator.h"

class ImageWindow
{
public:
	ImageWindow();

	void Render(FractalParams p, bool& changed_params);
	FractalParams GetParams() { return m_frac_params; }
	std::string GetDevice() { return m_generator.GetDevice(); };
private:
	bool LoadTexture(int image_width, int image_height);
	ImVec2 GetImageSize();

	GLuint m_image_texture = 0;
	bool m_texture_valid = false;
	FractalParams m_frac_params;
	ImGuiWindowFlags m_wnd_flags;
	ColourImage m_img;
	FractalGenerator m_generator;
};

