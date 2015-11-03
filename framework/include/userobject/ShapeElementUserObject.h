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

#ifndef SHAPEELEMENTUSEROBJECT_H
#define SHAPEELEMENTUSEROBJECT_H

#include "ElementUserObject.h"

//Forward Declarations
class ShapeElementUserObject;

template<>
InputParameters validParams<ShapeElementUserObject>();

/**
 * ElementUserObject class in which the _phi and _grad_phi shape function data
 * is available and correctly initialized. This enables the calculation of
 * Jacobian matrix contributions inside a UO
 */
class ShapeElementUserObject : public ElementUserObject
{
public:
  ShapeElementUserObject(const InputParameters & parameters);

protected:
  /// shape function values
  const VariablePhiValue & _phi;

  /// shape function gradients
  const VariablePhiGradient & _grad_phi;
};

#endif //SHAPEELEMENTUSEROBJECT_H
