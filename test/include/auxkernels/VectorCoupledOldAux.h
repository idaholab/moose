//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Assembly.h"
#include "AuxKernel.h"
#include "MooseArray.h"
#include "MooseTypes.h"
#include "SubProblem.h"

/**
 * Old coupled vector variable
 */

class VectorCoupledOldAux : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  VectorCoupledOldAux(const InputParameters & parameters);

protected:
  RealVectorValue computeValue() override;

  const VectorVariableValue & _vector;
};
