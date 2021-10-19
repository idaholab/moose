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

class PerpendicularElectricFieldInterface : public VectorInterfaceKernel
{
public:
  static InputParameters validParams();

  PerpendicularElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _free_charge;
  Real _primary_eps;
  Real _secondary_eps;

  RealVectorValue _u_perp;
  RealVectorValue _secondary_perp;

  RealVectorValue _phi_u_perp;
  RealVectorValue _phi_secondary_perp;
};
