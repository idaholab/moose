//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

class MassFluxPenalty : public ADDGKernel
{
public:
  static InputParameters validParams();

  MassFluxPenalty(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_x_neighbor;
  const ADVariableValue & _vel_y;
  const ADVariableValue & _vel_y_neighbor;
  const unsigned short _comp;
  const bool _matrix_only;
  const Real _gamma;
};
