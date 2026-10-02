//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/**
 * Applies a scalar cohesive flux and its derivative supplied as material properties.
 */
class SimpleCohesiveFlux : public InterfaceKernel
{
public:
  static InputParameters validParams();

  SimpleCohesiveFlux(const InputParameters & parameters);

protected:
  Real computeQpResidual(Moose::DGResidualType type) override;
  Real computeQpJacobian(Moose::DGJacobianType type) override;

  const MaterialProperty<Real> & _q;
  const MaterialProperty<Real> & _dq_djump;
};
