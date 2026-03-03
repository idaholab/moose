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
 * Adds a dirichlet BC for prescribing a velocity parallel to the boundary face normal direction;
 * this boundary condition is meant for the velocity component variables for the Navier-Stokes
 * momentum equation. A positive dirichlet value would denote outflow, while negative denotes inflow
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
