//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BulkMaterial.h"

/// This declares an instantiation of the BulkMaterial template for a computation that
/// outputs a single Real number on each quadrature point and requires data from one variable,
/// a RankTwoTensor material property, and a Real material property.
typedef BulkMaterial<
    // tuple representation
    TupleStd,
    // output data type
    Real,
    // gathered input data types:
    GatherVariable,
    GatherMatProp<RankTwoTensor>,
    GatherMatProp<Real>>

    BulkMaterialTestParent;

class BulkMaterialTest : public BulkMaterialTestParent
{
public:
  static InputParameters validParams();

  BulkMaterialTest(const InputParameters & params);

  virtual void bulkCompute();

  // /// access the serialized bulk data providedby the parent class
  // using BulkMaterialTestParent::_input_data;
  // using BulkMaterialTestParent::_output_data;
};
