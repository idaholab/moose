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
#include "ShapeUserObject.h"

// Forward Declarations
class ShapeElementUserObject;

template <>
InputParameters validParams<ShapeElementUserObject>();

/**
 * ElementUserObject class in which the _phi and _grad_phi shape function data
 * is available and correctly initialized on EXEC_NONLINEAR (the Jacobian calculation).
 * This enables the calculation of Jacobian matrix contributions inside a UO.
 *
 * \warning It is up to the user to ensure _fe_problem.currentlyComputingJacobian()
 *          returns true before utilizing the shape functions.
 */
class ShapeElementUserObject : public ShapeUserObject<ElementUserObject>
{
public:
  ShapeElementUserObject(const InputParameters & parameters);
};

#endif // SHAPEELEMENTUSEROBJECT_H
