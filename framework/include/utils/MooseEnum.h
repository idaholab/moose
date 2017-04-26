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
   * Constructor that takes a list of enumeration values, and a separate string to set a default for
   * this instance
   * @param names - a list of names for this enumeration
   * @param default_name - the default value for this enumeration instance
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   * it's range of
   *                       defined values.
   */
  MooseEnum(std::string names, std::string default_name = "", bool allow_out_of_range = false);

  /**
   * Copy Constructor for use when creating vectors of MooseEnums
   * @param other_enum - The other enumeration to copy state from
   */
  MooseEnum(const MooseEnum & other_enum);

  /**
   * Named constructor to build an empty MooseEnum with only the valid names
   * and the allow_out_of_range flag taken from another enumeration
   * @param other_enum - The other enumeration to copy the validity checking data from
   */
  static MooseEnum withNamesFrom(const MooseEnumBase & other_enum);

  virtual ~MooseEnum() = default;

  /**
   * Cast operators to make this object behave as value_types and std::string
   * these methods can be used so that this class behaves more like a normal value_type enumeration
   */
  operator int() const { return _current_id; }
  operator std::string() const { return _current_name_preserved; }

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
   * Assignment operators
   *  TODO: Perhaps we should implement an int assignment operator
   * @param name - a string representing one of the enumeration values.
   * @return A reference to this object for chaining
   */
  MooseEnum & operator=(const std::string & name);

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const override { return _current_id > INVALID_ID; }

  // InputParameters is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MooseEnum & obj)
  {
    out << obj._current_name_preserved;
    return out;
  }

  /// get the current value cast to the enum type T
  template <typename T>
  T getEnum() const;

protected:
  /// Check whether the current value is deprecated when called
  virtual void checkDeprecated() const override;

  void removeEnumerationName(std::string name) final override;

private:
  /**
   * Private constructor for use by libmesh::Parameters
   */
  MooseEnum();

  /**
   * Private constructor that can accept a MooseEnumBase for ::withOptionsFrom()
   * @param other_enum - MooseEnumBase type to copy names and out-of-range data from
   */
  MooseEnum(const MooseEnumBase & other_enum);

  /// The current id
  int _current_id;

  /// The corresponding name
  std::string _current_name;
  std::string _current_name_preserved;
};

template <typename T>
T
MooseEnum::getEnum() const
{
#ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
  static_assert(std::is_enum<T>::value == true,
                "The type requested from MooseEnum::getEnum must be an enum type!\n\n");
#endif
  return static_cast<T>(_current_id);
}

#endif // MOOSEENUM_H
