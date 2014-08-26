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

#ifndef VECTORMOOSEENUM_H
#define VECTORMOOSEENUM_H

#include "MooseEnumBase.h"

#include "libmesh/parameters.h"

#include <set>

typedef std::set<std::string>::const_iterator MooseEnumIterator;

/**
 * This is a "smart" enum class intended to replace many of the shortcomings in the C++ enum type
 * It should be initialized with a comma-separated list of strings which become the enum values.
 * You may also optionally supply numeric ints for one or more values similar to a C++ enum.  This
 * is done with the "=" sign. It can be used any place where an integer (switch statements), const char*
 * or std::string is expected.  In addition the InputParameters system has full support for this Enum type
 */
class MultiMooseEnum : public MooseEnumBase
{
public:
  /**
   * Constructor that takes a list of enumeration values, and a separate string to set a default for this instance
   * @param names - a list of names for this enumeration
   * @param default_name - the default value for this enumeration instance
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of it's range of
   *                       defined values.
   */
  MultiMooseEnum(std::string names, std::string default_names="", bool allow_out_of_range=false);

  /**
   * Copy Constructor
   * @param other_enum - The other enumeration to copy state from
   */
  MultiMooseEnum(const MultiMooseEnum & other_enum);

  virtual ~MultiMooseEnum();

  /**
   * Comparison operators for comparing with character constants, MultiMooseEnums
   * or integer values
   * @param value - RHS value to compare against
   * @return bool - the truth value for the comparison
   */
  bool operator==(const MultiMooseEnum & value) const;

  ///@{
  /**
   * Contains methods for seeing if a value is in the MultiMooseEnum.
   * @return bool - the truth value indicating whether the value is present
   */
  bool contains(const std::string & value) const;
  bool contains(int value) const;
  bool contains(unsigned short value) const;
  bool contains(const MultiMooseEnum & value) const;
  ///@}

  ///@{
  /**
   * Assignment operators
   * @param names - a string, set, or vector representing one of the enumeration values.
   * @return A reference to this object for chaining
   */
  MultiMooseEnum & operator=(const std::string & names);
  MultiMooseEnum & operator=(const std::vector<std::string> & names);
  MultiMooseEnum & operator=(const std::set<std::string> & names);
  ///@}

  ///@{
  /**
   * Insert operators
   * Operator to insert values into the enum. Existing values are preserved and
   * duplicates are discarded.
   * @param names - a string, set, or vector representing one of the enumeration values.
   */
  void insert(const std::string & names);
  void insert(const std::vector<std::string> & names);
  void insert(const std::set<std::string> & names);
  ///@}

  ///@{
  /**
   * Returns a begin/end iterator to all of the items in the enum. Items will
   * always be capitalized.
   */
  MooseEnumIterator begin() const { return _current_names.begin(); }
  MooseEnumIterator end() const { return _current_names.end(); }
  ///@}


  /**
   * Clear the MultiMooseEnum
   */
  void clear();

  /**
   * Return the number of items in the MooseEnum
   */
  unsigned int size() const { return _current_ids.size(); }

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const { return !_current_ids.empty(); }

  // InputParameters is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MultiMooseEnum & obj);

private:

  /**
   * Private constructor for use by libmesh::Parameters
   */
  MultiMooseEnum();

  MultiMooseEnum & assign(const std::set<std::string> &names, bool append);

  /// The current id
  std::set<int> _current_ids;

  /// The corresponding name
  std::set<std::string> _current_names;
  std::set<std::string> _current_names_preserved;
};

#endif //VECTORMOOSEENUM_H
