#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "Helper.hpp"
#include <random>
#include <list>
#include <algorithm>
#include <assert.h>

#define NUMBER_ROUTES (1000)
#define SELECTION_DROP_OFF_RATE (0.15)
#define NUMBER_GENERATIONS (400)
#define MUTATION_POSSIBILITY (0.15)

#define GOOD_ENOUGH_ROUTE_IN_KM ((double)10000)
#define SEARCH_FOR_OPTIMAL_SOLUTION (1)

#define ENABLE_PRINTING_STEPS (0)

double sum_all_routes_distances;
RouteAttributes best_route;
size_t best_route_generation;
size_t current_generation;

void generatePopulations(const std::map<size_t, City*>& cities, std::vector<RouteAttributes>& routes)
{
	size_t route_size = cities.size();
	std::random_device rd;
	std::mt19937 g(rd());

	for (size_t i = 0; i < NUMBER_ROUTES; i++)
	{
		std::vector<size_t> route;
		for (size_t i = 1; i < route_size + 1; i++)
			route.push_back(i);
		std::shuffle(route.begin(), route.end(), g);
		routes.push_back({i ,route, 0});
	}
}

void calculateSingleRouteDistance(const std::map<size_t, City*>& cities, RouteAttributes& route)
{
	size_t size = cities.size();
	route.distance_ = 0;
	for (size_t iter = 0; iter < size; iter++)
	{
		City* first = cities.at(route.route_.at(iter));
		City* second = nullptr;
		if (iter == size - 1)
			second = cities.at(route.route_.at(0));
		else	
			second = cities.at(route.route_.at(iter + 1));

		route.distance_ += Helper::calculateDistance(first, second);
	}
}

void calculateRouteDistances(const std::map<size_t, City*>& cities, std::vector<RouteAttributes>& routes)
{
	sum_all_routes_distances = 0;

	for (size_t i = 0; i < NUMBER_ROUTES; i++)
	{
		calculateSingleRouteDistance(cities, routes.at(i));
	
		if (best_route.distance_ == (double)0 || routes.at(i).distance_ < best_route.distance_)
		{
			best_route.distance_ = routes.at(i).distance_;
			best_route.route_ = routes.at(i).route_;
			best_route_generation = current_generation;
		}
		sum_all_routes_distances += routes.at(i).distance_;
	}
}

void fitnessFunction(std::vector<RouteAttributes>& routes, std::vector<RouteFitness>& route_fitness)
{
  	for (size_t i = 0; i < NUMBER_ROUTES; i++)
	{
		double percentage = routes.at(i).distance_ /  sum_all_routes_distances * 100;
		route_fitness.push_back({routes.at(i).id_, percentage});
	}
	if (ENABLE_PRINTING_STEPS)
		for (size_t i = 0; i < NUMBER_ROUTES; i++)
			std::cout << "FitnessFunction for route " << i << ":  " << route_fitness.at(i).fitness_percentage_ << " %" << std::endl;	
}

void selection(std::vector<RouteAttributes>& routes, std::vector<RouteFitness>& route_fitness)
{
	size_t number_routes_to_dismiss = (double)NUMBER_ROUTES * SELECTION_DROP_OFF_RATE;
	std::sort(route_fitness.begin(), route_fitness.end(), Helper::compare);
	
	if (ENABLE_PRINTING_STEPS)
		for (size_t i = 0; i < NUMBER_ROUTES; i++)
		{
			std::cout << "Sorted for route " << route_fitness.at(i).route_id_ << ":  ";
			std::cout<< route_fitness.at(i).fitness_percentage_ << " %" << std::endl;
		}

	size_t routes_in_better_half = (NUMBER_ROUTES - number_routes_to_dismiss) / 2;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(NUMBER_ROUTES - routes_in_better_half, NUMBER_ROUTES - 1);

	for (size_t i = 0; i < number_routes_to_dismiss; i++)
	{
		size_t route_to_dismiss = route_fitness.at(i).route_id_;
		if (ENABLE_PRINTING_STEPS)
			std::cout << "Removing the route with id : " << route_to_dismiss << std::endl;

		size_t index_to_select = dis(gen);

		size_t route_id = routes.at(route_to_dismiss).id_;
		routes.at(route_to_dismiss) = routes.at(route_fitness.at(index_to_select).route_id_);
		routes.at(route_to_dismiss).id_ = route_id;
	}
}

double calculatePartialDistance(const std::map<size_t, City*>& cities, const RouteAttributes& route, const size_t& index)
{
	City* current = cities.at(route.route_.at(index));
	City* previous = nullptr;
	City* next = nullptr;

	if (index == 0)
	{
		previous = cities.at(route.route_.at(cities.size() - 1));
		next = cities.at(route.route_.at(1));
	}
	else if (index == cities.size() - 1)
	{
		previous = cities.at(route.route_.at(index - 1));
		next = cities.at(route.route_.at(0));
	}
	else
	{
		previous = cities.at(route.route_.at(index - 1));
		next = cities.at(route.route_.at(index + 1));
	}
	
	return Helper::calculateDistance(previous, current) + Helper::calculateDistance(current, next);
}

bool ifSwitchingImproves(const std::map<size_t, City*>& cities, const RouteAttributes& route, const size_t& curr_index, const size_t& switch_pos_index)
{
	size_t current_distance = calculatePartialDistance(cities, route, curr_index) + calculatePartialDistance(cities, route, switch_pos_index);

	RouteAttributes route_copy = route;
	size_t temp = route_copy.route_.at(curr_index);
	route_copy.route_.at(curr_index) = route_copy.route_.at(switch_pos_index);
	route_copy.route_.at(switch_pos_index) = temp;

	size_t distance_after_switch = calculatePartialDistance(cities, route_copy, curr_index) + calculatePartialDistance(cities, route_copy, switch_pos_index);

	return distance_after_switch < current_distance;
}

void crossOver(std::vector<RouteAttributes>& routes, const std::map<size_t, City*>& cities)
{
	size_t num_cities = cities.size();

	for (size_t i = 0; i < NUMBER_ROUTES - 1; i += 2)
	{
		for (size_t index = 0; index < num_cities; index++)
		{
			const size_t first_city = routes.at(i).route_.at(index);
			const size_t sec_city = routes.at(i + 1).route_.at(index);

			size_t switch_city_pos = Helper::getCityPosition(routes.at(i).route_, sec_city);
			if (ifSwitchingImproves(cities, routes.at(i), index, switch_city_pos))
			{
				routes.at(i).route_.at(switch_city_pos) = first_city;
				routes.at(i).route_.at(index) = sec_city;
			}

			switch_city_pos = Helper::getCityPosition(routes.at(i + 1).route_, first_city);
			if (ifSwitchingImproves(cities, routes.at(i + 1), index, switch_city_pos))
			{
				routes.at(i + 1).route_.at(switch_city_pos) = sec_city;
				routes.at(i + 1).route_.at(index) = first_city;
			}
		}																
	}	
}

void shuffleRoutes(std::vector<RouteAttributes>& routes)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(routes.begin(), routes.end(), gen);
}

void mutation(std::vector<RouteAttributes>& routes, const std::map<size_t, City*>& cities)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, cities.size() - 1);

	for (size_t i = 0; i < NUMBER_ROUTES; i++)
	{
		if (Helper::getRandomNumber() <= 100 * MUTATION_POSSIBILITY)
		{
			size_t first_random_city = dis(gen);
			size_t second_random_city = dis(gen);
			while(second_random_city == first_random_city) // assuming there is more than 1 city
				second_random_city = dis(gen);

			double previous_distance = routes.at(i).distance_;

			size_t temp = routes.at(i).route_.at(first_random_city);
			routes.at(i).route_.at(first_random_city) = routes.at(i).route_.at(second_random_city);
			routes.at(i).route_.at(second_random_city) = temp;

			if (ENABLE_PRINTING_STEPS)
			{
				calculateSingleRouteDistance(cities, routes.at(i));
				std::cout << "Mutation effect (distance increased/decreased by): " << routes.at(i).distance_ - previous_distance << std::endl;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	std::map<size_t, City*> cities;
	if(argv[1] == nullptr)
		return -1;
	best_route.distance_ = 0;

	Helper::parseArguments(argv[1], cities);
	Helper::printCities(cities);

	std::vector<RouteAttributes> routes;
	generatePopulations(cities, routes);

	for (size_t i = 1; i < NUMBER_GENERATIONS + 1; i++)
	{
		current_generation = i;
		std::cout << "Generation: " << i << "  (total generations :" << NUMBER_GENERATIONS << ")" << std::endl;
		shuffleRoutes(routes);
		calculateRouteDistances(cities, routes);

		if (SEARCH_FOR_OPTIMAL_SOLUTION == 0 && best_route.distance_ <= GOOD_ENOUGH_ROUTE_IN_KM)
		{
			std::cout << "Found good enough route! " << std::endl;
			break;
		}
		std::vector<RouteFitness> route_fitness;
		fitnessFunction(routes, route_fitness);

		selection(routes, route_fitness);
		crossOver(routes, cities);
		mutation(routes, cities);
	}
	calculateRouteDistances(cities, routes);
	Helper::printBestRoute(cities, best_route);
	return 0;
}


