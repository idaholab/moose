//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "BoundaryFluxBase.h"

// Forward Declarations

/**
 * A boundary condition object for the advection equation
 * using a cell-centered finite volume method
 *
 * Notes:
 *
 *   1. This BC kernel itself does not do any complex calculation.
 *      It gets the flux vector and Jacobian matrix
 *      from the boundary flux user object being called.
 *
 *   2. If a system of governing equations is being solved,
 *      the flux vector and Jacobian matrix
 *      are calculated only once for the first equation
 *      and cached for use for the rest of the equations in the system.
 *
 *   3. On the "left" state of the boundary face, the variable value
 *      is interpolated from the reconstructed linear polynomial in the host element,
 *      which is provided from the corresponding material kernel.
 *
 *   4. On the "right" state of the boundary face, the variable value
 *      should be obtained from the bc user object being called.
 *
 */
class AEFVBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  AEFVBC(const InputParameters & parameters);
  virtual ~AEFVBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// choose an equation
  MooseEnum _component;

  // "1" denotes variable value from the host element

  /// piecewise constant variable values in host element
  const VariableValue & _uc1;

  /// extrapolated variable values at side center
  const MaterialProperty<Real> & _u1;

  /// bounadry flux object
  const BoundaryFluxBase & _flux;
};
