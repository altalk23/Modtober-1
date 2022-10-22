#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>

namespace tulip {
	class MouseManager {
	public:
		static bool containsPoint(cocos2d::CCNode* node, cocos2d::CCPoint const& mpos);
		static cocos2d::CCPoint getMousePosition();
	};
}