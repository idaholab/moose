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

class ADHDGAdvection : public ADDGKernel
{
public:
  static InputParameters validParams();

  ADHDGAdvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// The velocity on the element
  const ADMaterialProperty<RealVectorValue> & _velocity;
  /// The velocity on the neighbor
  const ADMaterialProperty<RealVectorValue> & _velocity_neighbor;

  const ADVariableValue & _side_u;
};
