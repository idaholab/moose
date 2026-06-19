//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

class INSPressureGradientDGKernel : public ADDGKernel
{
public:
  static InputParameters validParams();

  INSPressureGradientDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  // Coupled vars
  const ADVariableValue & _pressure;
  const ADVariableValue & _pressure_neighbor;

  // Required parameters
  const unsigned _component;
};
