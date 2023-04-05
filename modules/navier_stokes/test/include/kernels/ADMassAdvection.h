//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class ADMassAdvection : public ADKernel
{
public:
  static InputParameters validParams();

  ADMassAdvection(const InputParameters & parameters);

  virtual ~ADMassAdvection() {}

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const ADVariableValue & _vel_z;
  const ADVariableGradient & _grad_vel_x;
  const ADVariableGradient & _grad_vel_y;
  const ADVariableGradient & _grad_vel_z;
  const Moose::CoordinateSystemType & _coord_sys;
  const unsigned int _rz_radial_coord;
};
