#pragma once
#include <Utils/Types.h>
#include <Utils/Input.h>
#include <Camera/Camera.h>

namespace Phoenix
{
	class Scenario
	{
	public:
		virtual cstr_t Name() = 0;
		virtual ~Scenario() {}
		virtual bool Initialize() = 0;
		virtual void Run() = 0;

	public:
		virtual void MouseMove(const MouseInfo& info) = 0;
		virtual void MouseClick(const MouseInfo& info) = 0;
		virtual void MouseDrag(const MouseInfo& info) = 0;
		virtual void MouseWheel(const MouseInfo& info) = 0;
		virtual void Resize(uint32_t width, uint32_t height)=0;

	public:
		const Camera& GetMainCamera() { return mCamera; }

	protected:
		Camera mCamera;
	};
}
