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
 * Imposes Darcy and/or Forchheimer porous resistance on the momentum equation
 * using interstitial velocity (u = u_superficial / epsilon).
 */
class LinearFVMomentumPorousFriction : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearFVMomentumPorousFriction(const InputParameters & params);

protected:
  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;

  Real computeDarcyCoefficient(const Moose::ElemArg & elem_arg,
                               const Moose::StateArg & state) const;
  Real computeForchheimerCoefficient(const Moose::ElemArg & elem_arg,
                                     const Moose::StateArg & state) const;

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Mesh dimension
  const unsigned int _dim;

  /// Darcy coefficient (1/K), vector by component
  const Moose::Functor<RealVectorValue> * const _darcy;

  /// Forchheimer coefficient, vector by component
  const Moose::Functor<RealVectorValue> * const _forchheimer;

  /// Porosity
  const Moose::Functor<Real> & _eps;

  /// Dynamic viscosity (for Darcy)
  const Moose::Functor<Real> * const _mu;

  /// Density (for Forchheimer)
  const Moose::Functor<Real> * const _rho;

  /// Velocity components used to compute speed for Forchheimer
  const Moose::Functor<Real> * const _u;
  const Moose::Functor<Real> * const _v;
  const Moose::Functor<Real> * const _w;
};
