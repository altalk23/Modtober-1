#include <cocos2d.h>
#include <Geode/Loader.hpp>
#include "MouseManager.hpp"
#include <Geode/utils/operators.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/casts.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/MenuGameLayer.hpp>
#include <Geode/binding/GJDropDownLayer.hpp>

#include <cmath>
#include <numbers>

USE_GEODE_NAMESPACE();

namespace tulip {

	class Logic : public CCObject {
	public:
		static inline Logic* get() {
			auto ret = new Logic;
			return ret;
		}

		bool shouldIgnore(CCNode* node) {
			if (node->getContentSize() == CCSizeMake(0, 0)) return true;

			if (typeinfo_cast<CCMenu*>(node)) return true;
			return false;
		}

		void recurseOrMove(CCNode* node, CCPoint const& mousePosition) {
			if (typeinfo_cast<MenuGameLayer*>(node)) return;
			if (typeinfo_cast<GJDropDownLayer*>(node)) return;

			if (this->shouldIgnore(node)) {
				// probably has children that has content sizes, recurse
				if (node->getChildren()) {
					for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
						this->recurseOrMove(child, mousePosition);
					}
				}
			}
			else {
				auto size = node->getScaledContentSize();

			    auto pos = node->getParent()->convertToWorldSpace(node->getPosition());
			    
			    auto rect = CCRect {
			        pos.x - size.width / 2,
			        pos.y - size.height / 2,
			        size.width,
			        size.height
			    };

				if (rect.containsPoint(mousePosition)) {
					auto focalPoint1 = size.width < size.height ?
						// bottom focal
						CCPointMake(pos.x, rect.getMinY() + size.width / 2) :
						// left focal
						CCPointMake(rect.getMinX() + size.height / 2, pos.y);

					auto focalPoint2 = size.width < size.height ?
						// top focal
						CCPointMake(pos.x, rect.getMaxY() - size.width / 2) :
						// right focal
						CCPointMake(rect.getMaxX() - size.height / 2, pos.y);

					auto focalPoint = focalPoint1.getDistanceSq(mousePosition) < focalPoint2.getDistanceSq(mousePosition) ?
						focalPoint1 : focalPoint2; // which focal point is the closest

					if (size.width < size.height && 
						// between bottom and top
						focalPoint1.y < mousePosition.y && mousePosition.y < focalPoint2.y
					) {
						focalPoint.y = mousePosition.y;
					}

					if (size.width >= size.height && 
						// between left and right
						focalPoint1.x < mousePosition.x && mousePosition.x < focalPoint2.x
					) {
						focalPoint.x = mousePosition.x;
					}

					auto vec = mousePosition - focalPoint;
					auto angle = vec.getAngle();
					auto shortSide = size.width < size.height ? size.width : size.height;

					auto const pi = std::numbers::pi_v<float>;
					auto angleCheck = std::fmod(angle + 9 * pi / 4, 2 * pi);

					CCPoint borderPoint;

					if (angleCheck < pi / 2) {
						// right side
						auto side = shortSide / 2;
						borderPoint = CCPointMake(side, side * std::tanf(angle));
					}
					else if (angleCheck < pi) {
						// top side
						auto side = shortSide / 2;
						borderPoint = CCPointMake(side / std::tanf(angle), side);
					}
					else if (angleCheck < 3 * pi / 2) {
						// left side
						auto side = -shortSide / 2;
						borderPoint = CCPointMake(side, side * std::tanf(angle));
					}
					else {
						// bottom side
						auto side = -shortSide / 2;
						borderPoint = CCPointMake(side / std::tanf(angle), side);
					}

					auto moveVec = vec - borderPoint;

					auto newPos = pos + moveVec;
					if (newPos.x > CCDirector::get()->getScreenRight()) newPos.x = CCDirector::get()->getScreenRight();
					if (newPos.x < CCDirector::get()->getScreenLeft()) newPos.x = CCDirector::get()->getScreenLeft();
					if (newPos.y > CCDirector::get()->getScreenTop()) newPos.y = CCDirector::get()->getScreenTop();
					if (newPos.y < CCDirector::get()->getScreenBottom()) newPos.y = CCDirector::get()->getScreenBottom();
					auto newPosTransform = node->getParent()->convertToNodeSpace(newPos);
					node->setPosition(newPosTransform);
				}
				else {
					node->setRotation(0.0f);
				}

			}
		}

		void update(float dt) override {
			auto menuLayer = GameManager::get()->m_menuLayer;
			auto mousePosition = MouseManager::getMousePosition();
			if (menuLayer) {
				for (auto child : CCArrayExt<CCNode*>(menuLayer->getChildren())) {
					this->recurseOrMove(child, mousePosition);
				}
			}
		}
	};


}

GEODE_API bool GEODE_CALL geode_enable() {
	Loader::get()->queueInGDThread([](){
		CCDirector::get()->getScheduler()->scheduleUpdateForTarget(
			tulip::Logic::get(), 0, false
		);
	});

	return true;
}

GEODE_API bool GEODE_CALL geode_disable() {
	Loader::get()->queueInGDThread([](){
		CCDirector::get()->getScheduler()->unscheduleUpdateForTarget(
			tulip::Logic::get()
		);
	});

	return true;
}