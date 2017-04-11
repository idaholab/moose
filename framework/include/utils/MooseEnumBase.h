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

// C++ includes
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdarg>

// MOOSE includes
#include "MooseTypes.h"

/**
 * The base class for both the MooseEnum and MultiMooseEnum classes.
 */
class MooseEnumBase
{
public:

  /**
   * Constructor that takes a list of enumeration values, and a
   * separate string to set a default for this instance.
   * @param names - a list of names for this enumeration
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   *                             it's range of defined values.
   */
  MooseEnumBase(std::string names, bool allow_out_of_range = false);

  /**
   * Copy Constructor for use when creating vectors of MooseEnumBases
   * @param other_enum - The other enumeration to copy state from
   */
  MooseEnumBase(const MooseEnumBase & other_enum);

  /**
   * This class must have a virtual destructor since it has derived classes.
   */
  virtual ~MooseEnumBase() = default;

  /**
   * Deprecates various options in the MOOSE enum. For each deprecated option,
   * you may supply an option new option that will be used in a message telling
   * the user which new option replaces the old one.
   */
  void deprecate(const std::string & name, const std::string & new_name = "");

  /**
   * Method for returning a vector of all valid enumeration names for this instance
   * @return a vector of names
   */
  /// TODO: This should probably be turn into a set to avoid duplicate entries
  const std::vector<std::string> & getNames() const { return _names; }

  /**
   * Method for returning a set of all valid enumeration ids for this instance.
   * @returns A set of valid ids.
   */
  const std::set<int> & getIDs() const { return _ids; }

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

  /**
   * Return the id given a name.
   @returns The id of a valid enumeration.
   */
  int id(std::string name) const;

  /**
   * Return the name given a id.
   @returns The name of a valid enumeration.
   */
  const std::string & name(const int & id) const;

  /**
   * Adds possible enumeration name(s).
   */
  void addEnumerationNames(const std::string & names);
  void addEnumerationName(const std::string & name, const int & value);

  /**
   * Removes possible enumeration name(s.
   */
  void removeEnumerationNames(const std::string & names);

protected:
  MooseEnumBase();

  /**
   * Adds a possible enumeration value to the enum.
   * @param raw_name The enumeration name, which may include an id (e.g., foo=42).
   */
  void addEnumerationName(const std::string & raw_name);

  /**
   * Remove a possible enumeration value to the enum.
   * @param name The enumeration name to remove.
   */
  virtual void removeEnumerationName(std::string name);

  /**
   * Populates the _names vector
   * @param names - a space separated list of names used to populate the internal names vector
   */
  void fillNames(std::string names, std::string option_delim = " ");


  // The method that must be implemented to check derived class values against the _deprecated_names
  // list
  virtual void checkDeprecated() const = 0;

  /**
   * Check and warn deprecated values
   */
  void checkDeprecatedBase(const std::string & name_upper) const;

  /// The vector of enumeration names
  std::vector<std::string> _names;

  /// The raw string of names separated by spaces
  std::string _raw_names;

  /// The map of names to enumeration constants
  std::map<std::string, int> _name_to_id;

  /// The map of names to enumeration constants
  std::map<int, std::string> _id_to_name;

  /// The map of deprecated names and optional replacements
  std::map<std::string, std::string> _deprecated_names;

  /// Map id to raw name for removal
  std::map<std::string, std::string> _name_to_raw_name;

  /**
   * The index of values assigned that are NOT values in this enum.  If this index is 0 (false) then
   * out of range values are not allowed.
   */
  int _out_of_range_index;

  /// A set of ids utilized to check that it is not set the same for multiple keys
  std::set<int> _ids;

  /// Constants
  const static int INVALID_ID;
};

#endif // MOOSEENUMBASE_H
