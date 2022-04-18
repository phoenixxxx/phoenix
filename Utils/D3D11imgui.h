#pragma once
#include <Utils/Types.h>
#include <Utils/Singleton.h>
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_win32.h>
#include <ThirdParty/imgui/imgui_impl_dx11.h>
#include <memory>
#include <functional>
#include <D3D11/D3D11Manager.h>

namespace Phoenix
{
	class D3D11imgui
	{
	public:
		bool Initialize();
		void BeginRender();
		void EndRender();
		void Shutdown();

	public:
		uint32_t DrawComboBox(const stdstr_t& boxName, const std::vector<stdstr_t>& names, uint32_t selectedIndex);

		SINGLETON_METHODS(D3D11imgui)
	};
}