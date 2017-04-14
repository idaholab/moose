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

#ifndef MULTIMOOSEENUM_H
#define MULTIMOOSEENUM_H

// MOOSE includes
#include "MooseEnumBase.h"

// C++ includes
#include <set>

// Forward declarations
class MultiMooseEnum;
namespace libMesh
{
class Parameters;
}
namespace MooseUtils
{
MultiMooseEnum createExecuteOnEnum(const std::set<ExecFlagType> &,
                                   const std::set<ExecFlagType> &,
                                   const std::set<ExecFlagType> &);
}

typedef std::set<std::string>::const_iterator MooseEnumIterator;

/**
 * This is a "smart" enum class intended to replace many of the
 * shortcomings in the C++ enum type It should be initialized with a
 * comma-separated list of strings which become the enum values.  You
 * may also optionally supply numeric ints for one or more values
 * similar to a C++ enum.  This is done with the "=" sign. It can be
 * used any place where an integer (switch statements), const char* or
 * std::string is expected.  In addition the InputParameters system
 * has full support for this Enum type
 */
class MultiMooseEnum : public MooseEnumBase
{
public:

  /**
   * Constructor that takes a list or vector of enumeration values, and a separate string to set a
   * default for this instance
   * @param names - a list of names for this enumeration
   * @param default_names - the default value for this enumeration instance
   * @param allow_out_of_range - determines whether this enumeration will accept values outside of
   * it's range of defined values.
   */
  MultiMooseEnum(std::string names,
                 std::string default_names = "",
                 bool allow_out_of_range = false);

  /**
   * Copy Constructor
   * @param other_enum - The other enumeration to copy state from
   */
  MultiMooseEnum(const MultiMooseEnum & other_enum);

  /**
   * Named constructor to build an empty MultiMooseEnum with only the
   * valid names and the allow_out_of_range flag taken from another enumeration
   * @param other_enum - The other enumeration to copy the validity checking data from
   */
  static MultiMooseEnum withNamesFrom(const MooseEnumBase & other_enum);

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
   * Un-assign a value
   * @param names - a string, set, or vector giving the name to erase from the enumeration values
   */
  void erase(const std::string & names);
  void erase(const std::vector<std::string> & names);
  void erase(const std::set<std::string> & names);
  ///@}

  ///@{
  /**
   * Insert operators
   * Operator to insert (push_back) values into the enum. Existing values are preserved and
   * duplicates are stored.
   * @param names - a string, set, or vector representing one of the enumeration values.
   */
  void push_back(const std::string & names);
  void push_back(const std::vector<std::string> & names);
  void push_back(const std::set<std::string> & names);
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
   * Operator to retrieve an item from the MultiMooseEnum. The reference may not be used to change
   * the item.
   * @param i index
   * @returns a read/read-write reference to the item as an unsigned int.
   */
  unsigned int get(unsigned int i) const;

  ///@{
  /**
   * Returns a begin/end iterator to all of the items in the enum. Items will
   * always be capitalized.
   */
  MooseEnumIterator begin() const { return _current_names.begin(); }
  MooseEnumIterator end() const { return _current_names.end(); }
  ///@}

  /**
   * Get a list of the current valid enumeration.s
   */
  const std::set<std::string> & getCurrentNames() const { return _current_names; }

  /**
   * Get a list of the current valid enumeration.s
   */
  const std::vector<int> & getCurrentIDs() const { return _current_ids; }

  /**
   * Clear the MultiMooseEnum
   */
  void clear();

  /**
   * Return the number of items in the MultiMooseEnum
   */
  unsigned int size() const;

  /**
   * Returns the number of unique items in the MultiMooseEnum
   */
  unsigned int unique_items_size() const;

  /**
   * IsValid
   * @return - a Boolean indicating whether this Enumeration has been set
   */
  virtual bool isValid() const override { return !_current_ids.empty(); }

  void removeEnumerationName(std::string name) final override;

  // InputParameters and Output is allowed to create an empty enum but is responsible for
  // filling it in after the fact
  friend class libMesh::Parameters;

  // The create function can build an empty MultiMooseEnums for the execution flags.
  friend MultiMooseEnum MooseUtils::createExecuteOnEnum(const std::set<ExecFlagType> &,
                                                        const std::set<ExecFlagType> &,
                                                        const std::set<ExecFlagType> &);
  friend class SetupInterface;

  /// Operator for printing to iostreams
  friend std::ostream & operator<<(std::ostream & out, const MultiMooseEnum & obj);

protected:
  /// Check whether any of the current values are deprecated when called
  virtual void checkDeprecated() const override;

private:
  /**
   * Private constructor for use by libmesh::Parameters
   */
  MultiMooseEnum();

  /**
   * Private constructor that can accept a MooseEnumBase for ::withOptionsFrom()
   * @param other_enum - MooseEnumBase type to copy names and out-of-range data from
   */
  MultiMooseEnum(const MooseEnumBase & other_enum);

  /**
   * Helper method for all inserts and assignment operators
   */
  template <typename InputIterator>
  MultiMooseEnum & assign(InputIterator first, InputIterator last, bool append);

  /**
   * Helper method for un-assigning enumeration values
   */
  template <typename InputIterator>
  void remove(InputIterator first, InputIterator last);

  /// The current id
  std::vector<int> _current_ids;

  /// The corresponding name
  std::set<std::string> _current_names;
  std::vector<std::string> _current_names_preserved;
};

#endif // MULTIMOOSEENUM_H
