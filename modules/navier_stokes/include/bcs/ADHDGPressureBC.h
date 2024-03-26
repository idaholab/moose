//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class ADHDGPressureBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  ADHDGPressureBC(const InputParameters & parameters);

  virtual ~ADHDGPressureBC() {}

protected:
  virtual ADReal computeQpResidual();

  // Coupled vars
  const ADVariableValue & _pressure;

  // Required parameters
  const unsigned _component;
};
