//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdvectiveFluxCalculatorBase.h"

/**
 * Computes Advective fluxes for a constant velocity
 */
class AdvectiveFluxCalculatorConstantVelocity : public AdvectiveFluxCalculatorBase
{
public:
  static InputParameters validParams();

  AdvectiveFluxCalculatorConstantVelocity(const InputParameters & parameters);

protected:
  virtual Real computeVelocity(unsigned i, unsigned j, unsigned qp) const override;

  virtual Real computeU(unsigned i) const override;

  /// advection velocity
  RealVectorValue _velocity;

  /// the nodal values of u
  const VariableValue & _u_at_nodes;

  /// Kuzmin-Turek shape function
  const VariablePhiValue & _phi;

  /// grad(Kuzmin-Turek shape function)
  const VariablePhiGradient & _grad_phi;
};
