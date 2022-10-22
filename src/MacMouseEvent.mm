#include "MacMouseEvent.hmm"
#include "MouseManager.hpp"
#include <Geode/utils/operators.hpp>

#ifdef GEODE_IS_MACOS

#import <Geode/cocos/platform/mac/CCEventDispatcher.h>
#import <Geode/cocos/platform/mac/EAGLView.h>
#import <Foundation/Foundation.h>


USE_GEODE_NAMESPACE();

static MacMouseEvent* s_sharedEvent = nil;
using EventType = void(*)(id, SEL, NSEvent*);


@implementation MacMouseEvent


static EventType s_originalMouseMoved;
- (void)mouseMovedHook:(NSEvent*)event {
	s_originalMouseMoved(self, @selector(mouseMoved:), event);
	[[MacMouseEvent sharedEvent] moved:event];
}

+ (void)load {
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		Class class_ = NSClassFromString(@"EAGLView");

		Method mouseMovedMethod = class_getInstanceMethod(class_, @selector(mouseMoved:));
		Method mouseMovedHookMethod = class_getInstanceMethod([self class], @selector(mouseMovedHook:));
		s_originalMouseMoved = (decltype(s_originalMouseMoved))method_getImplementation(mouseMovedMethod);
		method_exchangeImplementations(mouseMovedMethod, mouseMovedHookMethod);
	});
}

+(MacMouseEvent*) sharedEvent {
	@synchronized(self) {
		if (s_sharedEvent == nil) {
			s_sharedEvent = [[self alloc] init]; // assignment not done here
		}
	}
	
	return s_sharedEvent;
}

-(void) moved:(NSEvent*)event {
	NSPoint event_location = [event locationInWindow];
	NSPoint local_point = [[NSClassFromString(@"EAGLView") sharedEGLView] convertPoint:event_location fromView:nil];
	
	float x = local_point.x;
	float y = [[NSClassFromString(@"EAGLView") sharedEGLView] getHeight] - local_point.y;

	m_xPosition = x / [[NSClassFromString(@"EAGLView") sharedEGLView] frameZoomFactor];
	m_yPosition = y / [[NSClassFromString(@"EAGLView") sharedEGLView] frameZoomFactor];
}

-(cocos2d::CCPoint) getMousePosition {
	return cocos2d::CCPoint(m_xPosition, m_yPosition);
}

@end

CCPoint tulip::MouseManager::getMousePosition() {
	static auto cachedMousePos = CCPointMake(0, 0);
	auto mpos = [s_sharedEvent getMousePosition];
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