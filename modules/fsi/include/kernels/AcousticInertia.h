//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "Material.h"

class AcousticInertia : public TimeKernel
{
public:
  static InputParameters validParams();

  AcousticInertia(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;

  Real computeQpJacobian() override;

private:
  const MaterialProperty<Real> & _inv_co_sq;
  const VariableValue & _u_dot_old;
  const VariableValue & _du_dot_du;
  const VariableValue & _du_dotdot_du;
  const VariableValue & _u_dot_factor;
  const VariableValue & _u_dotdot_factor;
};
