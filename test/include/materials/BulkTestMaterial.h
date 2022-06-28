//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "BulkMaterialTest.h"
#include "RankTwoTensor.h"

/**
 * Test material that goes with BulkMaterialTest
 */
class BulkTestMaterial : public Material
{
public:
  static InputParameters validParams();

  BulkTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  // used only for checking the bulk computation
  const VariableValue & _var1;
  const MaterialProperty<RankTwoTensor> & _prop1;
  const MaterialProperty<Real> & _prop2;

  // output bulk result as property
  MaterialProperty<Real> & _prop_out;

  // coupling the bulk computation result
  const BulkMaterialTest & _bulk_uo;
  const BulkMaterialTest::OutputVector & _output;
};
