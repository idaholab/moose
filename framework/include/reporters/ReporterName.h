//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include <iostream>
#include "MooseTypes.h"

class Parameter;

/**
 * The Reporter system is comprised of objects that can contain any number of data values. This
 * class is a wrapper that severs two main purposes.
 *
 * 1. It provides a single "name" for accessing the data. When developing the Reporter system the
 *    original implementation required this object to serve as a key in a std::unordered_map. That
 *    is no longer the case, but those items remain in case that changes again in the future.
 * 2. Provide a means for defining special Parser syntax to allow for a single input parameter
 *    to define both the object and value name. This is the primary reason for this class, please
 *    refer to Parser.C for the specialization.
 *
 * Examples:
 *   ReporterName("object", "data");
 *   ReporterName("object/data");
 *
 */
class ReporterName
{
public:
  ReporterName(const std::string & object_name, const std::string & value_name);
  ReporterName(const std::string & object_and_value_name);
  ReporterName(const ReporterName & other);
  ReporterName & operator=(const ReporterName & other);
  ReporterName() {} // empty constructor for InputParameters

  /**
   * Return the object name that produces the Reporter value
   */
  const std::string & getObjectName() const;

  /**
   * Return the data name for the Reporter value.
   */
  const std::string & getValueName() const;

  /**
   * Return the name of the object and data as object_name/data_name
   */
  const std::string & getCombinedName() const;

  /**
   * std::string operator allows this object to behave as a std::sting object
   */
  operator std::string() const;

  /**
   * Compare with another object or string
   */
  bool operator==(const ReporterName & rhs) const;
  bool operator==(const std::string & combined_name) const;

  /**
   * Less than operator
   */
  bool operator<(const ReporterName & rhs) const;

private:
  std::string _object_name;
  std::string _value_name;
  std::string _combined_name;
};

// This with the operator== allow this to be used as a key in a std::unordered_map
namespace std
{
template <>
struct hash<ReporterName>
{
  size_t operator()(const ReporterName & other) const { return hash<string>{}(other); }
};
}

// Support << output
std::ostream & operator<<(std::ostream & os, const ReporterName & state);
