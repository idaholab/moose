/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEENUM_H
#define MOOSEENUM_H

#include "parameters.h"

#include <string>
#include <vector>
#include <map>
#include <ostream>

/**
 * This is a "smart" enum class intended to replace many of the shortcomings in the C++ enum type
 * It should be initialized with a comma-seperated list of strings which become the enum values.
 * You may also optionally supply numeric ints for one or more values similar to a C++ enum.  This
 * is done with the "=" sign. It can be used any place where an integer (switch statements), const char*
 * or std::string is expected.  In addition the InputParameters system has full support for this Enum type
 */
class MooseEnum
{
public:
  /**
   * Constructor that takes a list of enumeration values but doesn't set a default for this instance
   * @param names - a list of names for this enumeration
   */
  MooseEnum(std::string names);

  /**
   * Constructor that takes a list of enumeration values, and a seperate string to set a default for this instance
   * @param names - a list of names for this enumeration
   * @default_name - the default value for this enumeration instance
   */
  MooseEnum(std::string names, std::string default_name);

  /**
   * Method for returning a vector of all valid enumeration names for this instance
   * @return a vector of names
   */
  /// TODO: This should probably be turn into a set to avoid duplicate entries
  const std::vector<std::string> & getNames() const { return _names; }

  /**
   * Method for returning the raw name strings for this instance
   * @return a comma seperated list of names
   */
  const std::string & getRawNames() const { return _raw_names; }

  /**
   * Method for returning the raw name strings for this instance
   * @return a space seperated list of names
   */
  const std::string & getRawNamesNoCommas() const { return _raw_names_no_commas; }

  /**
   * Cast operators to make this object behave as value_types and std::string
   * these methods can be used so that this class behaves more like a normal value_type enumeration
   */
  operator int() const { return _current_id; }
  operator std::string() const { return _current_name_preserved; }

  /**
   * Comparison operator for comparing with strings or character constants
   * @param name - character constant for comparing with this enumeration object
   * @return bool - the truth value for the comparison
   */
  bool operator==(const char * name) const;

  /**
   * Assignment operators
   *  TODO: Perhaps we should implement an int assignment operator
   * @param name - a string representing one of the enumeration values.
   * @return A reference to this object for chaining
   */
  MooseEnum & operator=(const std::string &name);

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  bool isValid() const { return _current_id > -std::numeric_limits<int>::max(); }

  // InputParameters is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MooseEnum & obj) { out << obj._current_name_preserved; return out; }

private:

  /**
   * Private constructor for use by libmesh::Parameters
   */
  MooseEnum();

  /**
   * Populates the _names vector
   * @param names - a space seperated list of names used to populate the internal names vector
   */
  void fillNames(std::string names);

  /// The vector of enumeration names
  std::vector<std::string> _names;

  /// The raw string of names sepearted by commas
  std::string _raw_names;

  /// The raw string of names seperated by space
  std::string _raw_names_no_commas;

  /// The map of names to enumeration constants
  std::map<std::string, int> _name_to_id;

  /// The current id
  int _current_id;

  /// The corresponding name
  std::string _current_name;
  std::string _current_name_preserved;
};

#endif //MOOSEENUM_H
