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

/// This declares an instantiation of the BatchMaterial template for a computation that
/// outputs a single Real number on each quadrature point and requires data from one variable,
/// a RankTwoTensor material property, and a Real material property.
typedef BatchMaterial<
    // tuple representation
    BatchMaterialUtils::TupleStd,
    // output data type
    Real,
    // gathered input data types:
    BatchMaterialUtils::GatherVariable,
    BatchMaterialUtils::GatherMatProp<RankTwoTensor>,
    BatchMaterialUtils::GatherMatProp<Real>>

    BatchMaterialTestParent;

class BatchMaterialTest : public BatchMaterialTestParent
{
public:
  static InputParameters validParams();

  BatchMaterialTest(const InputParameters & params);

  void batchCompute() override;
};
