//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "MooseTypes.h"

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

  std::vector<const VectorVariableValue *> _vectors;
};
