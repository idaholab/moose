//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalHydraulicsApp.h"

class ThermalHydraulicsApp;
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
   * Build a name from 2 strings and a number
   */
  std::string genName(const std::string & prefix, const std::string & name, unsigned int i) const
  {
    std::stringstream ss;
    ss << prefix << ":" << name << ":" << i;
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

  /**
   * Build a name from strings that is safe to use in input files (i.e. can be exposed to users)
   */
  std::string genSafeName(const std::string & prefix,
                          const std::string & middle,
                          const std::string & suffix = "") const
  {
    std::stringstream ss;
    ss << prefix << "_" << middle;
    if (!suffix.empty())
      ss << "_" << suffix;
    return ss.str();
  }
};
