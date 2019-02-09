#ifndef __GRID_H__
#define __GRID_H__

#include "cocos2d.h"
#include "Square.h"

USING_NS_CC;

class Grid : public cocos2d::Layer
{
public:
	virtual bool init();

	std::map<Point, Square*> squares;

	Menu * menu;

	float number_of_columns;
	float number_of_lines;

	Point getPositionOf(Point point);
	void setState(Square::State state, Point point);
	
	CREATE_FUNC(Grid);

protected:
	void drawLines();
};

#endif