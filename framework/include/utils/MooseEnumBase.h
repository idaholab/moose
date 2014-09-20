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

#ifndef MOOSEENUMBASE_H
#define MOOSEENUMBASE_H

#include "libmesh/parameters.h"

#include <string>
#include <vector>
#include <map>
#include <ostream>

class MooseEnumBase
{
public:
  /**
   * Constructor that takes a list of enumeration values, and a separate string to set a default for this instance
   * @param names - a list of names for this enumeration
   * @param default_name - the default value for this enumeration instance
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of it's range of
   *                       defined values.
   */
  MooseEnumBase(std::string names, bool allow_out_of_range=false);

  /**
   * Copy Constructor for use when creating vectors of MooseEnumBases
   * @param other_enum - The other enumeration to copy state from
   */
  MooseEnumBase(const MooseEnumBase & other_enum);

  /**
   * This class must have a virtual destructor since it has derived classes.
   */
  virtual ~MooseEnumBase();

  /**
   * Method for returning a vector of all valid enumeration names for this instance
   * @return a vector of names
   */
  /// TODO: This should probably be turn into a set to avoid duplicate entries
  const std::vector<std::string> & getNames() const { return _names; }

  /**
   * Method for returning the raw name strings for this instance
   * @return a space separated list of names
   */
  const std::string & getRawNames() const { return _raw_names; }

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const = 0;

protected:
  MooseEnumBase();

  /**
   * Populates the _names vector
   * @param names - a space separated list of names used to populate the internal names vector
   */
  void fillNames(std::string names, std::string option_delim=" ");

  /// The vector of enumeration names
  std::vector<std::string> _names;

  /// The raw string of names separated by spaces
  std::string _raw_names;

  /// The map of names to enumeration constants
  std::map<std::string, int> _name_to_id;

  /**
   * The index of values assigned that are NOT values in this enum.  If this index is 0 (false) then
   * out of range values are not allowed.
   */
  int _out_of_range_index;

  /// Constants
  const static int INVALID_ID;
};

#endif //MOOSEENUMBASE_H
