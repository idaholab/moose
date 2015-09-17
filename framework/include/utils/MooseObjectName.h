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
  MooseObjectName(const std::string & tag, const std::string & name, const std::string & sep = "::");

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

private:

  ///@{
  /// Storage for the various name compoments
  const std::string _tag;
  const std::string _name;
  const std::string _combined;
  const std::string _separator; // used for better error messages only
  ///@}
};
