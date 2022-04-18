#pragma once

namespace Phoenix
{
	enum class MouseButton { eMBNone = 0, eMBLeft, eMBRight, eMBCenter };
	enum SystemKeyBits { eSKShift = 1 << 0, eSKControl = 1 << 1, eSKAlt = 1<<2 };
	typedef unsigned int SystemKeys;
	struct MouseInfo
	{
		int x, y;//current position
		int dx, dy, dz;//drag
		MouseButton mButton;
		SystemKeys mModifier;
	};
}
