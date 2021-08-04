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
  ReporterName(const char * combined_name);
  ReporterName(){}; // empty constructor for InputParameters

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
  const std::string getCombinedName() const;

  /**
   * Return the name used for registration of this Reporter in the restartable data system.
   */
  std::string getRestartableName() const;

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

  /**
   * Converts the special type to a usable name for error reporting
   */
  std::string specialTypeToName() const;

  /**
   * @returns True if this ReporterName represents a Postprocessor
   */
  bool isPostprocessor() const { return _special_type == SpecialType::POSTPROCESSOR; }
  /**
   * @returns True if this ReporterName represents a VectorPostprocessor
   */
  bool isVectorPostprocessor() const { return _special_type == SpecialType::VECTORPOSTPROCESSOR; }

  /**
   * Sets the special type to a Postprocessor.
   *
   * See ReporterData::declareReporterValue.
   */
  void setIsPostprocessor() { _special_type = SpecialType::POSTPROCESSOR; }
  /**
   * Sets the special type to a VectorPostprocessor.
   *
   * See ReporterData::declareReporterValue.
   */
  void setIsVectorPostprocessor() { _special_type = SpecialType::VECTORPOSTPROCESSOR; }

  /**
   * Whether or not the ReporterName is empty, similar to std::string::empty()
   */
  bool empty() const { return _object_name.empty() || _value_name.empty(); }

private:
  /**
   * Enum for storing a "special" type for this Reporter.
   * This is used to designate Reporters that represent Postprocessors
   * and VectorPostprocessors in output and error handling.
   */
  enum class SpecialType
  {
    ANY = 0,
    POSTPROCESSOR = 1,
    VECTORPOSTPROCESSOR = 2
  };

  /// The "special" type for this Reporter, used for identifying Postprocesors and VectorPostprocessors.
  ReporterName::SpecialType _special_type = ReporterName::SpecialType::ANY;

  /// The object name
  std::string _object_name;
  /// The value name
  std::string _value_name;
};

/**
 * A ReporterName that represents a Postprocessor.
 */
class PostprocessorReporterName : public ReporterName
{
public:
  PostprocessorReporterName(const PostprocessorName & name);
};

/**
 * A ReporterName that represents a VectorPostprocessor.
 */
class VectorPostprocessorReporterName : public ReporterName
{
public:
  VectorPostprocessorReporterName(const VectorPostprocessorName & name,
                                  const std::string & vector_name);
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
