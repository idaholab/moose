//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"
#include "ComputeLagrangianObjectiveStress.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(NEML2StressToMOOSE, Material);
#else

#include "neml2/tensors/TensorBase.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

class ExecuteNEML2Model;

class NEML2StressToMOOSE : public ComputeLagrangianObjectiveStress
{
public:
  static InputParameters validParams();

  NEML2StressToMOOSE(const InputParameters & params);

  virtual void computeProperties() override;

protected:
  virtual void computeQpSmallStress() override;

  /// User object managing the execution of the NEML2 model
  const ExecuteNEML2Model & _execute_neml2_model;

  /// NEML2 batch index for the current element
  std::size_t _batch_index;

  /// labeled view of the small stress output
  const neml2::Tensor & _output_stress;
  /// labeled view of the small Jacobian output
  const neml2::Tensor & _output_jacobian;
};

#endif
