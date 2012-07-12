#ifndef __STOUT_STRINGIFY_HPP__
#define __STOUT_STRINGIFY_HPP__

#include <stdlib.h>

#include <iostream> // For 'std::cerr' and 'std::endl'.
#include <list>
#include <map>
#include <set>
#include <sstream> // For 'std::ostringstream'.
#include <string>

template <typename T>
std::string stringify(T t)
{
  std::ostringstream out;
  out << t;
  if (!out.good()) {
    std::cerr << "Failed to stringify!" << t << std::endl;
    abort();
  }
  return out.str();
}


template <typename T>
std::string stringify(const std::set<T>& set)
{
  std::ostringstream out;
  out << "{ ";
  typename std::set<T>::const_iterator iterator = set.begin();
  while (iterator != set.end()) {
    out << stringify(*iterator);
    if (++iterator != set.end()) {
      out << ", ";
    }
  }
  out << " }";
  return out.str();
}


template <typename T>
std::string stringify(const std::list<T>& list)
{
  std::ostringstream out;
  out << "[ ";
  typename std::list<T>::const_iterator iterator = list.begin();
  while (iterator != list.end()) {
    out << stringify(*iterator);
    if (++iterator != list.end()) {
      out << ", ";
    }
  }
  out << " ]";
  return out.str();
}


template <typename K, typename V>
std::string stringify(const std::map<K, V>& map)
{
  std::ostringstream out;
  out << "{ ";
  typename std::map<K, V>::const_iterator iterator = map.begin();
  while (iterator != map.end()) {
    out << stringify(*iterator.first);
    out << ": ";
    out << stringify(*iterator.second);
    if (++iterator != map.end()) {
      out << ", ";
    }
  }
  out << " }";
  return out.str();
}


#endif // __STOUT_STRINGIFY_HPP__
