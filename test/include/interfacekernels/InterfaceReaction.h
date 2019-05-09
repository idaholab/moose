//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

// Forward Declarations
class InterfaceReaction;

template <>
InputParameters validParams<InterfaceReaction>();

class InterfaceReaction : public InterfaceKernel
{
public:
  InterfaceReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _kf;
  Real _kb;
  const MaterialProperty<Real> & _D;
  const MaterialProperty<Real> & _D_neighbor;
};
