//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BatchMaterial.h"

template <typename OutputType, typename InputType>
using BatchPropertyDerivativeBase = BatchMaterial<
    // tuple representation
    BatchMaterialUtils::TupleStd,
    // output data type
    OutputType,
    // gathered input data types:
    BatchMaterialUtils::GatherMatProp<InputType>>;

template <typename OutputType, typename InputType>
class BatchPropertyDerivative : public BatchPropertyDerivativeBase<OutputType, InputType>
{
public:
  static InputParameters validParams();

  BatchPropertyDerivative(const InputParameters & params);

  void batchCompute() override {}
};

typedef BatchPropertyDerivative<RankTwoTensor, Real> BatchPropertyDerivativeRankTwoTensorReal;
