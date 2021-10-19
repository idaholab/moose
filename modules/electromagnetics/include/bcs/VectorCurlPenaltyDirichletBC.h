//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorIntegratedBC.h"

class VectorCurlPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorCurlPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  Real _penalty;
  const Function & _exact_x;
  const Function & _exact_y;
  const Function & _exact_z;
};
