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

#ifndef MOOSEOBJECTNAME_H
#define MOOSEOBJECTNAME_H

// STL includes
#include <string>

/**
 * A class for storing the names of MooseObject by tag and object name
 *
 * This class is used by the Control logic system, allowing for multiple tags
 * to be applied to many different MooseObjects.
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

  /// Allows for access for comparison operators
  friend class MooseObjectParameterName;

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
};

#endif // MOOSEOBJECTNAME_H
