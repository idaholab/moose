//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// C++ includes
#include <string>
#include <set>
#include <vector>
#include <map>

// MOOSE includes
#include "MooseEnumItem.h"

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
   *                             its range of defined values.
   */
  MooseEnumBase(std::string names, bool allow_out_of_range = false);

  /**
   * Copy Constructor for use when creating vectors of MooseEnumBases
   * @param other_enum - The other enumeration to copy state from
   */
  MooseEnumBase(const MooseEnumBase & other_enum);

  /**
   * Copy Assignment operator must be explicitly defined when a copy ctor exists and this
   * method is used.
   */
  MooseEnumBase & operator=(const MooseEnumBase & other_enum) = default;

  /**
   * This class must have a virtual destructor since it has derived classes.
   */
  virtual ~MooseEnumBase() = default;

  /**
   * Deprecates various options in the MOOSE enum. For each deprecated option,
   * you may supply an optional new option that will be used in a message telling
   * the user which new option replaces the old one.
   */
  virtual void deprecate(const std::string & name, const std::string & raw_name = "");

  /**
   * Method for returning a vector of all valid enumeration names for this instance
   * @return a vector of names
   */
  std::vector<std::string> getNames() const;

  /**
   * Method for returning the raw name strings for this instance
   * @return a space separated list of names
   */
  std::string getRawNames() const;

  /**
   * Method for returning a vector of ids for this instance
   * @return a vector of ints containing the possible ids for this enumeration
   */
  std::vector<int> getIDs() const;

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const = 0;

  /**
   * isOutOfRangeAllowed
   * @return - a Boolean indicating whether enum names out of range are allowed
   */
  bool isOutOfRangeAllowed() const { return _allow_out_of_range; }

  /**
   * Return the complete set of available flags.
   */
  const std::set<MooseEnumItem> & items() const { return _items; }

  /**
   * Add an item documentation string
   */
  void addDocumentation(const std::string & name, const std::string & doc);

  /**
   * Get the map containing each item's documentation string
   */
  const std::map<MooseEnumItem, std::string> & getItemDocumentation() const;

  ///@{
  /**
   * Locate an item.
   */
  std::set<MooseEnumItem>::const_iterator find(const MooseEnumItem & other) const;
  std::set<MooseEnumItem>::const_iterator find(const std::string & name) const;
  std::set<MooseEnumItem>::const_iterator find(int id) const;
  ///@}

  /**
   * Compute the next valid ID.
   */
  int getNextValidID() const;

  /**
   * Adds an enumeration item from name
   */
  MooseEnumBase & operator+=(const std::string & name);
  /**
   * Adds enumeration items from a list of names
   */
  MooseEnumBase & operator+=(const std::initializer_list<std::string> & names);

protected:
  MooseEnumBase();

  ///@{
  /**
   * Methods to add possible enumeration value to the enum.
   *
   * The MooseEnum/MultiMooseEnum are not designed to be modified, with respect to the list
   * of possible values. However, this is not the case for the ExecFlagEnum which is a special
   * type of MultiMooseEnum for managing the "execute_on" flags. These methods are used by
   * ExecFlagEnum to allow users to modify the available execute flags for their object.
   */
  void addEnumerationNames(const std::string & names);
  const MooseEnumItem & addEnumerationName(const std::string & raw_name);
  const MooseEnumItem & addEnumerationName(const std::string & name, const int & value);
  const MooseEnumItem & addEnumerationItem(const MooseEnumItem & item);
  ///@}

  /**
   * Method that must be implemented to check derived class values against the _deprecated_names
   */
  virtual void checkDeprecated() const = 0;

  /**
   * Check and warn deprecated values
   */
  void checkDeprecated(const MooseEnumItem & item) const;

  /// Storage for the assigned items
  std::set<MooseEnumItem> _items;

  /// The map of deprecated names and optional replacements
  std::map<MooseEnumItem, MooseEnumItem> _deprecated_items;

  /// Flag to enable enumeration items not previously defined
  bool _allow_out_of_range;

  /// The map of items and their respective documentation strings
  std::map<MooseEnumItem, std::string> _item_documentation;
};
