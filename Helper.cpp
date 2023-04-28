#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include "Helper.hpp"
#include <random>

extern size_t best_route_generation;

void Helper::parseArguments(char* filename, std::map<size_t, City*>& cities)
{
  std::ifstream file;
  file.open(filename);
  std::string line_of_file;
  if (file.fail())
  {
    printf("Opened file %s does not exist!\n", filename);
    exit(-1);
  }
  size_t counter = 0;
  while(std::getline(file, line_of_file))
  {
    if (counter == 0) // get rid of first line in the file
    {
      counter++;
      continue;
    }
    std::stringstream sstream(line_of_file);
    std::string token;
    std::vector<std::string> parsed_tokens;
    while(std::getline(sstream, token, ','))
    {
      parsed_tokens.push_back(token);
    }
    if (parsed_tokens.size() != 3)
    {
      printf("Invalid city description -> 3 arguments needed!\n");
      exit(-1);
    }
    
    City* new_city = new City;
    new_city->name_ = parsed_tokens.at(0);
    new_city->latitude_ = std::stod(parsed_tokens.at(1));
    new_city->longitude_ = std::stod(parsed_tokens.at(2));

    cities.insert({counter, new_city});
    counter++;
  }
  file.close();
}

size_t Helper::getCityPosition(const std::vector<size_t>& cities_of_route, const size_t& id_city)
{
  size_t size{ cities_of_route.size() };
  for (size_t i = 0; i < size; i++)
  {
    if (cities_of_route.at(i) == id_city)
      return i;
  }
  return -1; // should never happen
}

double Helper::calculateDistance(City* first, City* second)
{ 
  double diff_latitude = second->latitude_ * (M_PI / 180) - first->latitude_ * (M_PI / 180);
  double diff_longitude = second->longitude_ * (M_PI / 180) - first->longitude_ * (M_PI / 180);

  double a = pow(sin(diff_latitude /2), 2) + cos(first->latitude_ * (M_PI / 180)) * cos(second->latitude_ * (M_PI / 180)) * pow(sin(diff_longitude / 2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return EARTH_RADIUS * c;
}

bool Helper::compare(const RouteFitness& left, const RouteFitness& right)
{
  return left.fitness_percentage_ > right.fitness_percentage_;
}

// returns random number in range [1-100]
size_t Helper::getRandomNumber()
{
  static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 100);

  return dis(gen);
}

void Helper::printCities(const std::map<size_t, City*>& cities)
{
  for (auto i : cities)
  std::cout << i.first << "  (" << i.second->name_ << ": " << i.second->latitude_ << " | " << i.second->longitude_ << std::endl;

  std::cout << "Total cities: " << cities.size() << std::endl;
}

void Helper::printBestRoute(const std::map<size_t, City*>& cities, const RouteAttributes& route)
{
  std::cout << "\n-------PRINTING BEST ROUTE-------" << std::endl;
  for (const auto& city : route.route_)
    std::cout << cities.at(city)->name_ << " -> ";
  std::cout << cities.at(route.route_.at(0))->name_ << std::endl;
  std::cout << "\nDistance: "  << route.distance_ << " km" << std::endl;
  std::cout << "Best route is found in generation: " << best_route_generation << std::endl;

  if (STORE_RESULTS == 0)
    return;

  std::ofstream file;
  file.open("results.txt", std::ios::app);

  if (!file)
    exit(-2);

  file << "\n-------PRINTING BEST ROUTE-------" << std::endl;
  for (const auto& city : route.route_)
    file << cities.at(city)->name_ << " -> ";
  file << cities.at(route.route_.at(0))->name_ << std::endl;
  file << "\nDistance: "  << route.distance_ << " km" << std::endl;
  file << "Best route is found in generation: " << best_route_generation << std::endl;
  file << "-------BEST ROUTE END--------------\n\n" << std::endl;
}

void Helper::printRoute(const std::vector<size_t>& route)
{
  size_t size = route.size();
  std::cout << "---------ROUTE-----------" << std::endl;
  for(size_t j = 0; j < size; j++)
    std::cout << route.at(j) << " ";
  std::cout << route.at(0) << std::endl;
  std::cout << "\n--------------------------" << std::endl;
}
