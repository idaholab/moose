//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
/**
 * This class provides interface for extracting the periodic directions, residual, and jacobian
 * values from UserObjects associated with global strain calculation
 */
class GlobalStrainUserObjectInterface
{
public:
  virtual const RankTwoTensor & getResidual() const = 0;
  virtual const RankFourTensor & getJacobian() const = 0;
  virtual const VectorValue<bool> & getPeriodicDirections() const = 0;
};
