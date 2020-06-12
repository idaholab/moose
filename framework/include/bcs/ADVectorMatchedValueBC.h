//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorNodalBC.h"

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class ADVectorMatchedValueBC : public ADVectorNodalBC
{
public:
  static InputParameters validParams();

  ADVectorMatchedValueBC(const InputParameters & parameters);

protected:
  ADRealVectorValue computeQpResidual() override;

  const ADRealVectorValue & _v;
};
