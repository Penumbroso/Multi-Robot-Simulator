#include "RobotController.h"


void RobotController::onEnter()
{
	Node::onEnter();

	path_generator.setWorldSize({ grid->number_of_columns, grid->number_of_lines });
	path_generator.setHeuristic(AStar::Heuristic::manhattan);
	path_generator.setDiagonalMovement(false);
}

// Accordingly with the current state of the Robot, choose if the next destination is a package or a delivery
void RobotController::definePathOf(Robot * robot)
{
	if (robot->state == Robot::EMPTY && !grid->available_packages.empty())
	{
		robot->path = this->findShortestPath(robot->grid_coord, grid->available_packages);
		robot->destination = robot->path[0];
		robot->package = robot->destination;
		Util::removeIfContains<Point>(&grid->available_packages, robot->destination);
	}

	else if(robot->state == Robot::FULL)
	{
		robot->path = this->findShortestPath(robot->grid_coord, grid->ends);
		robot->destination = robot->path[0];
		robot->end = robot->destination;
	}

	else if (robot->state == Robot::EMPTY && grid->available_packages.empty()) 
	{
		// TODO: check for null grid->parking
		robot->path = this->findShortestPath(robot->grid_coord, {robot->start});
		robot->destination = robot->path[0];
	}
}

// Check if collision is Immminent, if so choose between two reactions:
// 1: dont move case the collidable robot is not in your path
// 2: move in case he is
void RobotController::preventCollisionOf(Robot * robot)
{
	auto next_position = robot->path.back();

	if (isCollisionImminent(next_position))
	{
		auto collision_robot = this->getRobotAt(next_position);
		auto path = collision_robot->path;

		if (!Util::contains<Point>(&path, robot->grid_coord))
		{
			robot->path.push_back(robot->grid_coord);
		}
		else
		{
			grid->static_collidables.push_back(next_position);
			robot->path = this->findShortestPath(robot->grid_coord, { robot->destination });
			grid->static_collidables.pop_back();

		}
		// TODO: add case where the collision_robot path is empty
	}
}

// Find the shortest path between a point and a list of destinations
// To do this he makes each path and chooses the shortest one between them.
vector<Point> RobotController::findShortestPath(Point origin, vector<Point> destinations) {
	vector<Point> shortest_path;
	int min_size = std::numeric_limits<int>::max();

	for (Point destination : destinations)
	{
		path_generator.clearCollisions();
		path_generator.addCollisions(grid->static_collidables);
		path_generator.removeCollision(destination);

		auto path = path_generator.findPath({ origin.x, origin.y }, { destination.x, destination.y });
		if (path.size() < min_size) {
			shortest_path = path;
			min_size = path.size();
		}
	}

	return shortest_path;
}

// Check to see if the next position that a robot is gonna move contains already another robot.
bool RobotController::isCollisionImminent(Point next_position)
{
	if (this->getRobotAt(next_position))
		return true;
	else
		return false;
}

// Simply gets a robot at an specifit grid position.
Robot * RobotController::getRobotAt(Point grid_position)
{
	for (auto robot : robots)
	{
		if (robot->grid_coord == grid_position)
			return robot;
	}
	return nullptr;
}