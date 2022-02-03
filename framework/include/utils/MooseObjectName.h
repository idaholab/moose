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
 * A class for storing the names of MooseObject by tag and object name.
 *
 * This class is used by the Control logic system, allowing for multiple tags
 * to be applied to many different MooseObjects.
 *
 * There are multiple ways to create an MooseObjectName object, the best being to input the
 * tag and object name as separate inputs. However, this is not always practically if the supplied
 * name is coming from an input file. Therefore, you can use the following single string methods.
 *
 * MooseObjectName(foo::bar)    -> tag="foo", name="bar"
 * MooseObjectName(foo/bar)     -> tag="foo", name="bar"
 * MooseObjectName(foo/foo/bar) -> tag="foo/foo", name="bar"
 *
 * This class also allows for glob style '*' to be used to allow for fuzzy comparisons to be
 * performed.
 *
 * MooseObjectName name1("*", "bar");
 * MooseObjectName name2("foo", "bar");
 * name1 == name2 (True)
 *
 * MooseObjectName name3("foo", "*");
 * MooseObjectName name4("foo", "bar");
 * name3 == name4 (True)
 */
class MooseObjectName
{
public:
  /**
   * Construct the name object.
   * @param tag The tag to apply the object
   * @param name The name of the object
   */
  MooseObjectName(const std::string & tag,
                  const std::string & name,
                  const std::string & separator = std::string("/"));

  /**
   * This class requires a virtual (but default) desctructor since it
   * has virtual functions.
   */
  virtual ~MooseObjectName() = default;

  /**
   * Build an object given a raw parameter name (e.g., from an input file parameter)
   */
  MooseObjectName(std::string name);

  /**
   * Copy constructor.
   */
  MooseObjectName(const MooseObjectName & rhs);

  /**
   * Return the name.
   */
  const std::string & name() const { return _name; }

  /**
   * Return the tag.
   */
  const std::string & tag() const { return _tag; }

  ///@{
  /**
   * Comparison operators.
   *
   * The less than operator is required so this container can work in std::map.
   *
   * @see InputParameterWarehouse
   */
  bool operator==(const MooseObjectName & rhs) const;
  bool operator!=(const MooseObjectName & rhs) const;
  bool operator<(const MooseObjectName & rhs) const;
  ///@}

  /// Allows this to be used with std:: cout
  friend std::ostream & operator<<(std::ostream & stream, const MooseObjectName & obj);

protected:
  /**
   * A constructor for use by MooseObjectParameterName
   */
  MooseObjectName();

  ///@{
  /// Storage for the various name components
  std::string _tag;
  std::string _name;
  std::string _combined;
  std::string _separator; // used for better error messages only
  ///@}

  /**
   * Check that the name and tag are supplied correctly.
   */
  virtual void check();
};
