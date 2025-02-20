//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Imposes a friction force on the momentum equation
 */
class LinearFVMomentumFriction : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearFVMomentumFriction(const InputParameters & params);

protected:
  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;
  Real computeFrictionWCoefficient(const Moose::ElemArg & elem_arg, const Moose::StateArg & state);

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Darcy coefficient
  const Moose::Functor<RealVectorValue> * const _D;
  /// Booleans to select the right models
  const bool _use_Darcy_friction_model;

  /// Dynamic viscosity
  const Moose::Functor<Real> * const _mu;
};
