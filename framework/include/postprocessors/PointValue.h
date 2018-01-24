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

#ifndef POINTVALUE_H
#define POINTVALUE_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class PointValue;

template <>
InputParameters validParams<PointValue>();

/**
 * Compute the value of a variable at a specified location.
 *
 * Warning: This postprocessor may result in undefined behavior if utilized with
 * non-continuous elements and the point being located lies on an element boundary.
 */
class PointValue : public GeneralPostprocessor
{
public:
  PointValue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  /// The variable number of the variable we are operating on
  const unsigned int _var_number;

  /// A reference to the system containing the variable
  const System & _system;

  /// The point to locate
  const Point & _point;

  /// The value of the variable at the desired location
  Real _value;
};

#endif /* POINTVALUE_H */
