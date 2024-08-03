//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseEnumBase.h"
#include "MooseError.h"

// C++ includes
#include <vector>

// Forward declarations
class ExecFlagEnum;
namespace libMesh
{
class Parameters;
}

typedef std::vector<MooseEnumItem>::const_iterator MooseEnumIterator;

/**
 * This is a "smart" enum class intended to replace many of the
 * shortcomings in the C++ enum type. There are two parts to a MultiMooseEnum:
 * 1) The list of all possible values that a variable can have and
 * 2) The subset of these values that the variable is actually set to.
 * It should be initialized with a comma-separated list of strings which
 * become the possible enum values. You may also optionally supply numeric
 * ints for one or more values similar to a C++ enum. This is done with the
 * "=" sign. It can be used any place where an integer (switch statements),
 * const char* or * std::string is expected. In addition the
 * InputParameters system has full support for this Enum type.
 */
class MultiMooseEnum : public MooseEnumBase
{
public:
  ///@{
  /**
   * Constructor that takes a list of valid enumeration values, and a separate string to set default
   * values for this instance
   * @param valid_names - a list of all possible values (names) for this enumeration
   * @param initialization_values - the value(s) to set this enumeration instance to
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   * its range of initially defined values. A value of true allows the addition of valid
   * values to an object after it has been initialized.
   */
  MultiMooseEnum(std::string valid_names,
                 std::string initialization_values,
                 bool allow_out_of_range = false);
  // We need to explicitly define this version so MultiMooseEnum("one two", "one")
  // doesn't implicitly convert "one" to a bool and use the 2-parameter constructor
  MultiMooseEnum(std::string valid_names,
                 const char * initialization_values,
                 bool allow_out_of_range = false);
  ///@}

  /**
   * Constructor that takes a list of valid enumeration values, and does not set this instance to
   * any particular value.
   * @param valid_names - a list of all possible values (names) for this enumeration
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   * its range of initially defined values. A value of true allows the addition of valid
   * values to an object after it has been initialized.
   */
  MultiMooseEnum(std::string valid_names, bool allow_out_of_range = false);

  /**
   * Copy Constructor
   * @param other_enum - The other enumeration to copy state from
   */
  MultiMooseEnum(const MultiMooseEnum & other_enum);

  /**
   * Copy Assignment operator must be explicitly defined when a copy ctor exists and this
   * method is used.
   */
  MultiMooseEnum & operator=(const MultiMooseEnum & other_enum) = default;

  ///@{
  /**
   * Comparison operators for comparing with character constants, MultiMooseEnums
   * or integer values
   * @param value - RHS value to compare against
   * @return bool - the truth value for the comparison
   */
  bool operator==(const MultiMooseEnum & value) const;
  bool operator!=(const MultiMooseEnum & value) const;
  ///@}

  ///@{
  /**
   * Methods for seeing if a value is set in the MultiMooseEnum.
   * @return bool - the truth value indicating whether the value is set
   */
  bool contains(const std::string & value) const { return isValueSet(value); }
  bool contains(int value) const { return isValueSet(value); }
  bool contains(unsigned short value) const { return isValueSet(value); }
  bool contains(const MultiMooseEnum & value) const { return isValueSet(value); }
  bool contains(const MooseEnumItem & value) const { return isValueSet(value); }
  ///@}

  // The following are aliases for contains with more descriptive name
  ///@{
  /**
   * Methods for seeing if a value is set in the MultiMooseEnum.
   * @return bool - the truth value indicating whether the value is set
   */
  bool isValueSet(const std::string & value) const;
  bool isValueSet(int value) const;
  bool isValueSet(unsigned short value) const;
  bool isValueSet(const MultiMooseEnum & value) const;
  bool isValueSet(const MooseEnumItem & value) const;
  ///@}

  ///@{
  /**
   * Assignment operators to set the objects value from the list of possible values.
   * @param names - a string, set, or vector representing one of the enumeration values.
   * @return A reference to this object for chaining
   */
  MultiMooseEnum & operator=(const std::string & names);
  MultiMooseEnum & operator=(const std::vector<std::string> & names);
  MultiMooseEnum & operator=(const std::set<std::string> & names);
  ///@}

  ///@{
  /**
   * Un-assign, or unset a value. Deprecated, use eraseSetValue instead.
   * @param names - a string, set, or vector giving the name to erase from the enumeration values
   */
  void erase(const std::string & names);
  void erase(const std::vector<std::string> & names);
  void erase(const std::set<std::string> & names);
  ///@}

  // The following replaces erase with a more descriptive name
  ///@{
  /**
   * Un-assign, or unset a value
   * @param names - a string, set, or vector giving the name to erase from the enumeration values
   */
  void eraseSetValue(const std::string & names);
  void eraseSetValue(const std::vector<std::string> & names);
  void eraseSetValue(const std::set<std::string> & names);
  ///@}

  ///@{
  /**
   * Insert operators
   * Operator to insert (push_back) values into the enum. Existing values are preserved and
   * duplicates are stored. Deprecated, use setAdditionalValue instead.
   * @param names - a string, set, or vector representing the enumeration values to set.
   */
  void push_back(const std::string & names);
  void push_back(const std::vector<std::string> & names);
  void push_back(const std::set<std::string> & names);
  void push_back(const MultiMooseEnum & other_enum);
  ///@}

  // The following replaces push_back with a more descriptive name
  ///@{
  /**
   * Insert operators
   * Operator to insert (push_back) values into the enum. Existing values are preserved and
   * duplicates are stored.
   * @param names - a string, set, or vector representing the enumeration values to set.
   */
  void setAdditionalValue(const std::string & names);
  void setAdditionalValue(const std::vector<std::string> & names);
  void setAdditionalValue(const std::set<std::string> & names);
  void setAdditionalValue(const MultiMooseEnum & other_enum);
  ///@}

  /**
   * Indexing operator
   * Operator to retrieve an item from the MultiMooseEnum. The reference may not be used to change
   * the item.
   * @param i index
   * @returns a read/read-write reference to the item as a string.
   */
  const std::string & operator[](unsigned int i) const;

  /**
   * Indexing operator
   * Operator to retrieve the id of an item from the MultiMooseEnum.
   * @param i index corresponding to the desired item
   * @returns the id of the MooseEnumItem at the supplied index
   */
  unsigned int get(unsigned int i) const;

  /// get the current values cast to a vector of enum type T. Deprecated, use getSetValueIDs instead.
  template <typename T>
  std::vector<T> getEnum() const;

  // The following replaces getEnum with a more descriptive name
  /// get the current values cast to a vector of enum type T
  template <typename T>
  std::vector<T> getSetValueIDs() const;

  ///@{
  /**
   * Returns a begin/end iterator to all of the set values in the enum.
   * Items will always be capitalized.
   */
  MooseEnumIterator begin() const { return _current_values.begin(); }
  MooseEnumIterator end() const { return _current_values.end(); }
  ///@}

  ///@{
  /**
   * Clear the MultiMooseEnum
   */
  void clearSetValues();
  void clear();
  ///@}

  /**
   * Return the number of active items in the MultiMooseEnum
   */
  unsigned int size() const;

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const override { return !_current_values.empty(); }

  // InputParameters and Output is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MultiMooseEnum & obj);

  // The following functions would add possible values
  // (to _items) that an enumerated variable can take. However "+=" is not a
  // descriptive enough funtion name for this and should not be used in case
  // users get confused that the operator actually changes the set values of the
  // variable (_current_values). We have re-implemented this logic under a method
  // with a more descirptive name to force users to be informed. These are deprecated.
  MooseEnumBase & operator+=(const std::string & name);
  MooseEnumBase & operator+=(const std::initializer_list<std::string> & names);

  // The following replaces operator+= with a more descriptive name
  /// Extends the range of possible values the variable can be set to
  void addValidName(const std::string & name);
  void addValidName(const std::initializer_list<std::string> & names);

protected:
  /// Check whether any of the current values are deprecated when called
  virtual void checkDeprecated() const override;

  /**
   * Helper method for all inserts and assignment operators
   * @param first An iterator specifying the beginning of the range of values to set
   * @param last An iterator specifying the end of the range of values to set
   */
  template <typename InputIterator>
  MultiMooseEnum & assignValues(InputIterator first, InputIterator last, bool append);

  /**
   * Helper method for un-assigning enumeration values
   * @param first An iterator specifying the beginning of the range of values to set
   * @param last An iterator specifying the end of the range of values to set
   */
  template <typename InputIterator>
  void removeSetValues(InputIterator first, InputIterator last);

  /// The current value(s) of the MultiMooseEnum.
  std::vector<MooseEnumItem> _current_values;

  /**
   * Protected constructor for use by libmesh::Parameters
   */
  MultiMooseEnum();

  /**
   * Protected constructor that can accept a MooseEnumBase for ::withOptionsFrom()
   * @param other_enum - MooseEnumBase type to copy names and out-of-range data from
   */
  MultiMooseEnum(const MooseEnumBase & other_enum);
};

template <typename T>
std::vector<T>
MultiMooseEnum::getSetValueIDs() const
{
#ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
  static_assert(std::is_enum<T>::value == true,
                "The type requested from MooseEnum::getEnum must be an enum type!\n\n");
#endif
  std::vector<T> enum_vec;
  for (const auto & current_value : _current_values)
    enum_vec.push_back(static_cast<T>(current_value.id()));
  return enum_vec;
}

template <typename T>
std::vector<T>
MultiMooseEnum::getEnum() const
{
  mooseDeprecated("MultiMooseEnum::getEnum is deprecated, use MultiMooseEnum::getSetValueIDs");
  return MultiMooseEnum::getSetValueIDs<T>();
}
