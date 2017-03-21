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

#ifndef MOOSEOBJECTPARAMETERNAME_H
#define MOOSEOBJECTPARAMETERNAME_H

// MOOSE includes
#include "MooseObjectName.h"

// STL includes
#include <string>

/**
 * A class for storing an input parameter name.
 *
 * This class is used by the Control logic system, allowing for multiple tags
 * to be applied to many different MooseObject parameters.
 */
class MooseObjectParameterName : public MooseObjectName
{
public:
  /**
   * Build an object given a raw parameter name (e.g., from an input file parameter)
   */
  MooseObjectParameterName(std::string name);

  /**
   * Build an object given a MooseObjectName and parameter name
   */
  MooseObjectParameterName(const MooseObjectName & obj_name, std::string param);

  /**
   * Return the parameter name.
   */
  const std::string & parameter() const { return _parameter; }

  ///@{
  /**
   * Comparison operators.
   *
   * Not that this class may be compared with MooseObjectName, this
   * feature is used by ControlInterface.
   *
   * The less than operator is required so this container can work in std::map.
   *
   * @see InputParameterWarehouse
   */
  bool operator==(const MooseObjectParameterName & rhs) const;
  bool operator==(const MooseObjectName & rhs) const;

  bool operator!=(const MooseObjectParameterName & rhs) const;
  bool operator!=(const MooseObjectName & rhs) const;

  bool operator<(const MooseObjectParameterName & rhs) const;
  ///@}

  // Allow printing with std:: cout
  friend std::ostream & operator<<(std::ostream & stream, const MooseObjectParameterName & obj);

protected:
  /// The name of the input parameter
  std::string _parameter;
};

#endif // MOOSEOBJECTPARAMETERNAME_H
