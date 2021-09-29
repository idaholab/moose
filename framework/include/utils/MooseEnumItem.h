//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// STL includes
#include <string>

/**
 * Class for containing MooseEnum item information.
 */
class MooseEnumItem
{
public:
  static const int INVALID_ID;
  MooseEnumItem();
  MooseEnumItem(const std::string & name, const int & id = INVALID_ID);
  ~MooseEnumItem() = default;
  MooseEnumItem(const MooseEnumItem & other);
  MooseEnumItem(MooseEnumItem && /*other*/) = default;
  MooseEnumItem & operator=(const MooseEnumItem & other);
  MooseEnumItem & operator=(MooseEnumItem && /*other*/) = default;

  ///@{
  /**
   * Return the numeric, name, or raw name.
   */
  const int & id() const { return _id; }
  const std::string & name() const { return _name; }
  const std::string & rawName() const { return _raw_name; }
  ///@}

  ///@{
  /**
   * Operator to allow this class to be used directly as a string for int.
   */
  operator int() const { return _id; }
  operator std::string() const { return _name; }
  ///@}

  ///@{
  /**
   * Comparison operators.
   *
   * The comparison operators using the char * and string are case sensitive.
   */
  bool operator==(const char * value) const;
  bool operator!=(const char * value) const;

  bool operator==(const std::string & value) const;
  bool operator!=(const std::string & value) const;

  bool operator==(int value) const { return _id == value; }
  bool operator!=(int value) const { return _id != value; }

  bool operator==(unsigned short value) const { return _id == value; }
  bool operator!=(unsigned short value) const { return _id != value; }

  bool operator==(const MooseEnumItem &) const;
  bool operator!=(const MooseEnumItem &) const;
  ///@}

  /**
   * Less than operator. This is required for this class to work in maps and sets.
   */
  bool operator<(const MooseEnumItem & other) const
  {
    return _id != other._id ? _id < other._id : _name < other._name;
  }

  /**
   * ostream operator for string printing.
   */
  friend std::ostream & operator<<(std::ostream & out, const MooseEnumItem & item);

  /**
   * Method to change the ID of the item, but only if it is an INVALID_ID. An error is produced
   * if the ID is valid and this method is called.
   *
   * This is needed to allow ExecFlagType objects to be created without an ID being provided, the
   * ID is assigned when ExecFlagEnum::addAvailableFlags is called.
   */
  void setID(const int & id);

private:
  /// The name as provided in constructor
  std::string _raw_name;

  /// Upper case name
  std::string _name;

  /// The numeric value for item
  int _id;
};
