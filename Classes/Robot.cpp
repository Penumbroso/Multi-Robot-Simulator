#include "Robot.h"
#include "Util.h"

USING_NS_CC;

bool Robot::init()
{
	if (!Sprite::init())
		return false;
	
	stopwatch = Stopwatch::create();
	this->addChild(stopwatch);

	this->scheduleUpdate();
	return true;
}

void Robot::move()
{
	Vector<FiniteTimeAction*> movements;

	auto origin = this->getPosition();
	reverse(screen_path.begin(), screen_path.end());

	auto callbackUpdateGridPosition = CallFunc::create(CC_CALLBACK_0(Robot::updateGridPosition, this));

	for (auto destination : screen_path)
	{
		double distance = sqrt(pow((destination.x - origin.x), 2.0) + pow((destination.y - origin.y), 2.0));

		float moveDuration = 0.01*distance;

		auto moveToNextSquare = MoveTo::create(moveDuration, destination);
		movements.pushBack(moveToNextSquare);
		movements.pushBack(callbackUpdateGridPosition);
		origin = destination;
	}

	auto callbackFinishedMovement = CallFunc::create(CC_CALLBACK_0(Robot::finishedMovement, this));

	auto sequence_of_movements = Sequence::create(movements);
	this->movement = Sequence::create(sequence_of_movements, callbackFinishedMovement, nullptr);
	runAction(movement);
}

void Robot::stop()
{
	this->stopAction(this->movement);
	this->unscheduleUpdate();
}

void Robot::updateState()
{
	if (getPosition() == screen_package)
		state = FULL;
	
	if (getPosition() == screen_delivery_point)
		state = EMPTY;
}

bool Robot::isParked()
{
	return grid_coord == grid_start;
}

bool Robot::isAtDeliverty()
{
	return getPosition() == screen_delivery_point;
}

bool Robot::isFull()
{
	return state == FULL;
}

bool Robot::isAtPackage()
{	
	return getPosition() == screen_package;
}

bool Robot::isInThe(vector<Point> path)
{
	return Util::contains<Point>(&path, this->grid_coord);
}

void Robot::update(float dt)
{
	if (getNumberOfRunningActions() == 0 && !screen_path.empty())
		this->move();
}

void Robot::updateGridPosition()
{
	
	grid_coord = grid_path.back();
	grid_path.pop_back();
}

void Robot::finishedMovement()
{
	if (this->isAtDeliverty()) 
	{
		EventCustom event("robot_at_delivery");
		event.setUserData(this);
		_eventDispatcher->dispatchEvent(&event);
	}
	else if (this->isAtPackage())
	{
		EventCustom event("robot_at_package");
		event.setUserData(this);
		_eventDispatcher->dispatchEvent(&event);
	}


	grid_path.clear();
	screen_path.clear();
	updateState();
}

