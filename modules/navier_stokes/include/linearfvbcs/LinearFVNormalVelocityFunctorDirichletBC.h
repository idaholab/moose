//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"

/**
 * Class implementing a Dirichlet boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVNormalVelocityFunctorDirichletBC : public LinearFVAdvectionDiffusionFunctorDirichletBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVNormalVelocityFunctorDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

protected:
  /// The velocity component this Dirichlet condition object is for
  const enum Component : int { X = 0, Y, Z } _component;
};
