//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "ShapeUserObject.h"

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
  static InputParameters validParams();

  ShapeElementUserObject(const InputParameters & parameters);
};
