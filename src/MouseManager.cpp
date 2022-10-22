#include "MouseManager.hpp"
#include <Geode/utils/operators.hpp>

USE_GEODE_NAMESPACE();

bool tulip::MouseManager::containsPoint(CCNode* node, cocos2d::CCPoint const& mpos) {
    auto size = node->getScaledContentSize();

    if (size == CCSizeMake(0, 0)) return false;

    auto pos = node->getParent()->convertToWorldSpace(node->getPosition());
    
    auto rect = CCRect {
        pos.x - size.width / 2,
        pos.y - size.height / 2,
        size.width,
        size.height
    };

    return rect.containsPoint(mpos);
}


#ifdef GEODE_IS_WINDOWS

CCPoint tulip::MouseManager::getMousePosition() {
    static auto cachedMousePos = CCPointMake(0, 0);
    auto mpos = CCDirector::sharedDirector()->getOpenGLView()->getMousePosition();
    if (mpos == cachedMousePos) return cachedMousePos;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto winSizePx = CCDirector::sharedDirector()->getOpenGLView()->getViewPortRect();
    auto ratio_w = winSize.width / winSizePx.size.width;
    auto ratio_h = winSize.height / winSizePx.size.height;
    mpos.y = winSizePx.size.height - mpos.y;
    mpos.x *= ratio_w;
    mpos.y *= ratio_h;
    return mpos;
}

#endif