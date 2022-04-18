#include "D3D11imgui.h"
#include <Window/Window.h>

namespace Phoenix
{
	bool D3D11imgui::Initialize()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(Window::Instance()->GetWindowHandle());
		ImGui_ImplDX11_Init(D3D11Manager::Instance()->GetDevice(), D3D11Manager::Instance()->GetContext());

		return true;
	}

	void D3D11imgui::Shutdown()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void D3D11imgui::BeginRender()
	{
		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::Begin("Phoenix", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	}

	void D3D11imgui::EndRender()
	{
		ImGui::End();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	uint32_t D3D11imgui::DrawComboBox(const stdstr_t& boxName, const std::vector<stdstr_t>& names, uint32_t selectedIndex)
	{
		uint32_t index = selectedIndex;
		if (ImGui::BeginCombo(boxName.c_str(), names[index].c_str(), ImGuiComboFlags_HeightSmall))
		{
			for (int n = 0; n < names.size(); n++)
			{
				const bool isSelected = (index == n);
				if (ImGui::Selectable(names[n].c_str(), isSelected))
					index = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return index;
	}
}
