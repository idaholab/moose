#pragma once

#include "THMApp.h"

class THMApp;
class NamingInterface;

/**
 * Interface for handling names
 */
class NamingInterface
{
public:
  NamingInterface() {}

  /**
   * Build a name from a prefix, number and possible suffix
   */
  std::string
  genName(const std::string & prefix, unsigned int id, const std::string & suffix = "") const
  {
    std::stringstream ss;
    ss << prefix << ":" << id;
    if (!suffix.empty())
      ss << ":" << suffix;
    return ss.str();
  }

  /**
   * Build a name from a prefix, 2 numbers and possible suffix
   */
  std::string genName(const std::string & prefix,
                      unsigned int i,
                      unsigned int j,
                      const std::string & suffix = "") const
  {
    std::stringstream ss;
    ss << prefix << ":" << i << ":" << j;
    if (!suffix.empty())
      ss << ":" << suffix;
    return ss.str();
  }

  /**
   * Build a name from strings
   */
  std::string genName(const std::string & prefix,
                      const std::string & middle,
                      const std::string & suffix = "") const
  {
    std::stringstream ss;
    ss << prefix << ":" << middle;
    if (!suffix.empty())
      ss << ":" << suffix;
    return ss.str();
  }
};
