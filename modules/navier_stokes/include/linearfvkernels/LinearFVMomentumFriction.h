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
 * Imposes Darcy and/or Forchheimer resistance on the momentum equation.
 */
class LinearFVMomentumFriction : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearFVMomentumFriction(const InputParameters & params);

protected:
  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;
  Real computeFrictionCoefficient(const Moose::ElemArg & elem_arg,
                                  const Moose::StateArg & state) const;

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Darcy coefficient
  const Moose::Functor<RealVectorValue> * const _D;

  /// Forchheimer coefficient
  const Moose::Functor<RealVectorValue> * const _F;

  /// Dynamic viscosity
  const Moose::Functor<Real> * const _mu;

  /// Density
  const Moose::Functor<Real> * const _rho;

  /// Porosity
  const Moose::Functor<Real> & _porosity;

  /// Mesh dimension
  const unsigned int _dim;

  /// Velocity components used to evaluate the Forchheimer speed
  const Moose::Functor<Real> * const _u;
  const Moose::Functor<Real> * const _v;
  const Moose::Functor<Real> * const _w;
};
