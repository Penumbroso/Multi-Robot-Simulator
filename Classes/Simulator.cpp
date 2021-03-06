#include "Simulator.h"
#include "SimpleAudioEngine.h"
#include "AStar.hpp"
#include <algorithm>

USING_NS_CC;

bool Simulator::init()
{    
	if (!Scene::initWithPhysics())
	{
		return false;
	}

	speed_multiplier = 1.0f;

	grid = Grid::create();
	stopwatch = Stopwatch::create();
	infobar = Infobar::create();
	toolbar = Toolbar::create();
	actionbar = Actionbar::create();
	robotController = RobotController::create();

	addChild(grid);
	addChild(stopwatch);
	addChild(infobar);
	addChild(toolbar);
	addChild(actionbar);
	addChild(robotController);

	setup();

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(Simulator::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	createCustomEvents();

    return true;
}

void Simulator::setCallbacks()
{
	actionbar->runItem->setCallback(CC_CALLBACK_1(Simulator::menuRunCallback, this));
	actionbar->exportItem->setCallback(CC_CALLBACK_1(Simulator::menuExportCallback, this));
	actionbar->resetItem->setCallback(CC_CALLBACK_1(Simulator::menuResetCallback, this));
	actionbar->speedUpItem->setCallback(CC_CALLBACK_0(Simulator::menuChangeSpeedCallback, this, 1/2.0));
	actionbar->slowDownItem->setCallback(CC_CALLBACK_0(Simulator::menuChangeSpeedCallback, this, 2.0));
	actionbar->moveItem->setCallback(CC_CALLBACK_1(Simulator::menuMoveGridCallback, this));
	actionbar->zoomInItem->setCallback(CC_CALLBACK_0(Simulator::menuZoomCallback, this, 1.1));
	actionbar->zoomOutItem->setCallback(CC_CALLBACK_0(Simulator::menuZoomCallback, this, 1/1.1));

	for (const auto &p : grid->squares)
	{
		auto square = p.second;
		square->setCallback(CC_CALLBACK_0(Simulator::gridSquareCallback, this, square->grid_position));
	}
}

void Simulator::createCustomEvents()
{
	auto robotAtDeliveryListener = EventListenerCustom::create("robot_at_delivery", CC_CALLBACK_1(Simulator::robotIsAtDelivery, this));
	_eventDispatcher->addEventListenerWithSceneGraphPriority(robotAtDeliveryListener, this);

	auto robotAtPackageListener = EventListenerCustom::create("robot_at_package", CC_CALLBACK_1(Simulator::robotIsAtPackage, this));
	_eventDispatcher->addEventListenerWithSceneGraphPriority(robotAtPackageListener, this);

	auto robotIsParkedListener = EventListenerCustom::create("robot_is_parked", CC_CALLBACK_1(Simulator::robotIsParked, this));
	_eventDispatcher->addEventListenerWithSceneGraphPriority(robotIsParkedListener, this);
}

void Simulator::setup()
{
	grid->setPosition(30, 0);
	infobar->time = &stopwatch->text;
	robotController->grid = grid;

	setCallbacks();
}

void Simulator::start()
{
	createRobots();

	for (auto robot : robots)
	{
		robotController->definePathOf(robot);
		robot->move();
	}
		
	robotController->robots = robots;
	stopwatch->setSpeedMultiplier(speed_multiplier);
	stopwatch->start();
}

void Simulator::stop()
{
	stopwatch->stop();
	for (auto robot : robots)
	{
		robot->pause();
	}
}

void Simulator::proceed()
{
	stopwatch->start();
}

void Simulator::createRobots() {
	if (!robots.empty()) return;

	for (Point start : grid->starts) 
	{
		auto robot = Robot::create();
		robot->initWithFile("Robot.png");
		robot->setPosition(grid->getPositionOf(start));
		robot->setColor(Color3B(150, 150, 150));
		robot->setContentSize(Size(grid->square_size, grid->square_size));
		robot->grid_position = start;
		robot->grid_start = start;

		auto physicsBody = PhysicsBody::createBox(Size(60.0f, 60.0f), PhysicsMaterial(0.1f, 1.0f, 0.0f));
		physicsBody->setGravityEnable(false);
		physicsBody->setDynamic(true);
		physicsBody->setContactTestBitmask(true);
		robot->addComponent(physicsBody);

		grid->addChild(robot);

		robots_bodies[physicsBody] = robot;
		robots.push_back(robot);
	}
}

bool Simulator::allPackagesWereDelivered()
{
	return grid->packages.size() == packages_delivered.size();
}

bool Simulator::allRobotsAreParked()
{
	for (auto robot : robots) 
	{
		if (!robot->isParked())
			return false;
	}
	return true;
}

void Simulator::reset()
{
	for (Point package : grid->packages)
		grid->setState(Square::PACKAGE, package);
	
	grid->available_packages = grid->packages;

	infobar->time = &stopwatch->text;
	for (auto robot : robots)
		robot->removeFromParentAndCleanup(true);
	
	robots.clear();
	packages_delivered.clear();
	stopwatch->reset();
}

void Simulator::menuRunCallback(cocos2d::Ref * pSender)
{
	start();
}

void Simulator::menuResetCallback(cocos2d::Ref * pSender)
{
	stop();
	reset();
}

void Simulator::gridSquareCallback(Point coord)
{
	auto robot = robotController->getRobotAt(coord);
	switch (toolbar->selected)
	{
	case Toolbar::PACKAGE:
		grid->setState(Square::PACKAGE, coord);
		break;

	case Toolbar::BEGIN:
		grid->setState(Square::BEGIN, coord);
		break;

	case Toolbar::END:
		grid->setState(Square::END, coord);
		break;

	case Toolbar::ERASE:
		grid->setState(Square::EMPTY, coord);
		break;

	case Toolbar::BLOCKADE:
		grid->setState(Square::BLOCKADE, coord);
		break;
	case Toolbar::PATH:
		if (robot) 
		{
			for (auto coord : robot->complete_grid_path)
				grid->squares[coord]->setColor(Color3B::RED);
		}
		break;
	case Toolbar::CLOCK:
		if (robot)
		{
			auto robot_time = robot_times[robot];
			std::string time = robot_time.getCString();
			infobar->time = &time;
		}
		break;
	}

}

void Simulator::menuExportCallback(cocos2d::Ref * pSender)
{
	std::ofstream out("times.txt");

	out << "Total time: ";
	out << stopwatch->toString();
	out << std::endl;

	int id = 0;
	for (auto robot : robots)
	{
		out << "id: ";
		out << id;
		out << " time: ";
		out << robot_times[robot].getCString();
		out << std::endl;
		id++;
	}

	out.close();
}

void Simulator::menuChangeSpeedCallback(float multiplier)
{
	stop();
	speed_multiplier *= multiplier;
	infobar->updateSpeed(1 / speed_multiplier);
	start();
}

void Simulator::menuMoveGridCallback(cocos2d::Ref * pSender)
{
	grid->toggleDragAndDrop();
}

void Simulator::menuZoomCallback(float multiplier)
{
	const float scale = grid->getScale();
	grid->setScale(scale * multiplier);
	infobar->updateZoom(scale * multiplier);

}

bool Simulator::onContactBegin(PhysicsContact & contact)
{
	auto bodyA = contact.getShapeA()->getBody();
	auto bodyB = contact.getShapeB()->getBody();

	auto r1 = robots_bodies[bodyA];
	auto r2 = robots_bodies[bodyB];

	//r1->stop();

	//robotController->repath(r1, r2);

	//r1->move();

	return false;
}

void Simulator::robotIsAtDelivery(EventCustom* event)
{
	const Robot* robot = static_cast<Robot*>(event->getUserData());
	Point grid_package = grid->getGridPositionOf(robot->screen_package);
	Util::addIfUnique<Point>(&packages_delivered, grid_package);
}

void Simulator::robotIsAtPackage(EventCustom* event)
{
	const Robot* robot = static_cast<Robot*>(event->getUserData());
	Point grid_package = grid->getGridPositionOf(robot->screen_package);
	grid->setState(Square::EMPTY, grid_package);
}

void Simulator::robotIsParked(EventCustom* event)
{
	Robot* robot = static_cast<Robot*>(event->getUserData());
	robot_times[robot] = stopwatch->toString();

	if (allRobotsAreParked() && allPackagesWereDelivered())
		stop();
}
