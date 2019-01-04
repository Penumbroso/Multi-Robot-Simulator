#include "Gap.h"

using namespace cocos2d;

void Gap::onEnter()
{
	Sprite::onEnter();
	// Register Touch Event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);

	listener->onTouchBegan = CC_CALLBACK_2(Gap::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(Gap::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

}

Rect Gap::getRect()
{
	auto s = getTexture()->getContentSize();
	return Rect(-s.width / 2, -s.height / 2, s.width, s.height);
}

bool Gap::containsTouchLocation(Touch* touch)
{
	return getRect().containsPoint(convertTouchToNodeSpaceAR(touch));
}

bool Gap::onTouchBegan(Touch* touch, Event* event)
{
	CCLOG("Paddle::onTouchBegan id = %d, x = %f, y = %f", touch->getID(), touch->getLocation().x, touch->getLocation().y);

	if (!containsTouchLocation(touch)) return false;

	return true;
}

bool Gap::onTouchEnded(Touch* touch, Event* event)
{
	CCLOG("Touch ended.");
	this->setColor(Color3B::GRAY);
	return true;
}

