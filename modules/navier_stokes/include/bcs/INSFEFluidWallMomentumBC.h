//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidIntegratedBCBase.h"

/**
 * Implicitly sets normal component of velocity to zero if the advection term of the momentum
 * equation is integrated by parts
 */
class INSFEFluidWallMomentumBC : public INSFEFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  INSFEFluidWallMomentumBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _mu_t;
  unsigned _component;
};
