//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class PotentialAdvection : public Kernel
{
public:
  static InputParameters validParams();

  PotentialAdvection(const InputParameters & parameters);
  virtual ~PotentialAdvection();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  const unsigned int _potential_id;
  const Real _sgn;
  VariableGradient _default;
  const VariableGradient & _grad_potential;
};
