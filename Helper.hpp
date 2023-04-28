#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#define STORE_RESULTS (0)

struct City
{
  std::string name_;
  double latitude_;
  double longitude_;
};

struct RouteAttributes 
{
  size_t id_;
  std::vector<size_t> route_;
  double distance_;
};
 
struct RouteFitness
{
  size_t route_id_; 
  double fitness_percentage_;
};

const double EARTH_RADIUS = 6373.0;

struct Helper
{
  static void parseArguments(char* filename, std::map<size_t, City*>& cities);
  static size_t getCityPosition(const std::vector<size_t>& cities_of_route, const size_t& id_city);
  static double calculateDistance(City* first, City* second);
  static bool compare(const RouteFitness& left, const RouteFitness& right);
  static void printCities(const std::map<size_t, City*>& cities);
  static void printBestRoute(const std::map<size_t, City*>& cities, const RouteAttributes& route);
  static void printRoute(const std::vector<size_t>& route);
  static size_t getRandomNumber();
};

#endif // HELPER_HPP