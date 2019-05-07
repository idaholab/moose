//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayIntegratedBC.h"

class ArrayVacuumBC;

template <>
InputParameters validParams<ArrayVacuumBC>();

class ArrayVacuumBC : public ArrayIntegratedBC
{
public:
  ArrayVacuumBC(const InputParameters & parameters);

protected:
  virtual RealArrayValue computeQpResidual() override;
  virtual RealArrayValue computeQpJacobian() override;
  virtual RealArray computeQpOffDiagJacobian(MooseVariableFEBase & jvar) override;

  /// Ratio of u to du/dn
  RealArrayValue _alpha;
};
