#pragma once
#include <vector>
#include <memory>
#include "Scenario.h"
#include <SharedGPU_CPU/SharedGeometry.h>
#include <Utils/VectorMath.h>
#include <BVH/BVH.h>
#include <BVHLighting/BVHLighting.h>
#include <Image/Image.h>
#include <SkyIllumination/Sky.h>

namespace Phoenix
{
	class MaterialEditor : public Scenario
	{
	public:
		MaterialEditor();
	public:
		virtual cstr_t Name();
		virtual ~MaterialEditor();
		virtual bool Initialize();
		virtual void Run();

	public:
		virtual void MouseMove(const MouseInfo& info);
		virtual void MouseClick(const MouseInfo& info);
		virtual void MouseDrag(const MouseInfo& info);
		virtual void MouseWheel(const MouseInfo& info);

		virtual void Resize(uint32_t width, uint32_t height);

	private:
		const float mCameraSpeed = 1.0f / 100;
		void DrawUI();
	private:
		uint32_t mMouseClickX, mMouseClickY;
	};
}