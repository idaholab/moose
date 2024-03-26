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

class MassFluxPenaltyBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  MassFluxPenaltyBC(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const unsigned short _comp;
  const bool _matrix_only;
};
