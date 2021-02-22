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
#include "MooseEnumBase.h"

// Forward declarations
namespace libMesh
{
class Parameters;
}

/**
 * This is a "smart" enum class intended to replace many of the
 * shortcomings in the C++ enum type It should be initialized with a
 * space-delimited list of strings which become the enum values.  You
 * may also optionally supply numeric ints for one or more values
 * similar to a C++ enum.  This is done with the "=" sign (no
 * spaces). It can be used any place where an integer (switch
 * statements), const char* or std::string is expected. In addition
 * the InputParameters system has full support for this Enum type
 */
class MooseEnum : public MooseEnumBase
{
public:
  /**
   * Enum item for controlling comparison in the compareCurrent method.
   */
  enum class CompareMode
  {
    COMPARE_NAME,
    COMPARE_ID,
    COMPARE_BOTH
  };

  /**
   * Constructor that takes a list of enumeration values, and a separate string to set a default for
   * this instance
   * @param names - a list of names for this enumeration
   * @param default_name - the default value for this enumeration instance
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   * it's range of defined values.
   */
  MooseEnum(std::string names, std::string default_name = "", bool allow_out_of_range = false);

  /**
   * Copy Constructor for use when creating vectors of MooseEnums
   * @param other_enum - The other enumeration to copy state from
   */
  MooseEnum(const MooseEnum & other_enum);

  /**
   * Copy Assignment operator must be explicitly defined when a copy ctor exists and this
   * method is used.
   */
  MooseEnum & operator=(const MooseEnum & other_enum) = default;

  virtual ~MooseEnum() = default;

  /**
   * Cast operators to make this object behave as value_types and std::string
   * these methods can be used so that this class behaves more like a normal value_type enumeration
   */
  operator int() const { return _current.id(); }
  operator std::string() const { return _current.rawName(); }

  /**
   * Comparison operators for comparing with character constants, MooseEnums
   * or integer values
   * @param value - RHS value to compare against
   * @return bool - the truth value for the comparison
   */
  bool operator==(const char * value) const;
  bool operator!=(const char * value) const;

  bool operator==(int value) const;
  bool operator!=(int value) const;

  bool operator==(unsigned short value) const;
  bool operator!=(unsigned short value) const;

  bool operator==(const MooseEnum & value) const;
  bool operator!=(const MooseEnum & value) const;

  /**
   * Method for comparing currently set values between MooseEnum.
   */
  bool compareCurrent(const MooseEnum & other, CompareMode mode = CompareMode::COMPARE_NAME) const;

  ///@{
  /**
   * Assignment operators/methods
   * @param name/int - a string or int representing one of the enumeration values.
   * @return A reference to this object for chaining
   */
  MooseEnum & operator=(const std::string & name);
  MooseEnum & operator=(int value);
  MooseEnum & operator=(const MooseEnumItem & item);
  void assign(const std::string & name);
  void assign(int value);
  void assign(const MooseEnumItem & item);
  ///@}

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const override { return _current.id() > MooseEnumItem::INVALID_ID; }

  // InputParameters is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MooseEnum & obj)
  {
    out << obj._current.rawName();
    return out;
  }

  /// get the current value cast to the enum type T
  template <typename T>
  T getEnum() const;

protected:
  /// Check whether the current value is deprecated when called
  virtual void checkDeprecated() const override;

  /**
   * Constructor for use by libmesh::Parameters and ReporterMode
   */
  MooseEnum();

private:
  /// The current id
  MooseEnumItem _current;
};

template <typename T>
T
MooseEnum::getEnum() const
{
#ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
  static_assert(std::is_enum<T>::value == true,
                "The type requested from MooseEnum::getEnum must be an enum type!\n\n");
#endif
  return static_cast<T>(_current.id());
}
