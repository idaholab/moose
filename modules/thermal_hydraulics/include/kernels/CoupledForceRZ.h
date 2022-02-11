//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CoupledForce.h"
#include "RZSymmetry.h"

/**
 * Source term proportional to the coupled variable in RZ coordinates
 */
class CoupledForceRZ : public CoupledForce, public RZSymmetry
{
public:
  CoupledForceRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

public:
  static InputParameters validParams();
};
