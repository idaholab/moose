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
#include "BatchMaterialParameterMap.h"
#include "RankTwoTensor.h"

/**
 * Test material that goes with BatchMaterialParameterMap user object
 */
class BatchMaterialParameterMapMaterial : public Material
{
public:
  static InputParameters validParams();

  BatchMaterialParameterMapMaterial(const InputParameters & parameters);

protected:
  void computeProperties() override;

  const MaterialProperty<Real> & _prop1;

  // output batch result as property
  MaterialProperty<Real> & _prop_out;

  // coupling the batch computation result
  const BatchMaterialParameterMap & _batch_uo;
  const BatchMaterialParameterMap::OutputVector & _output;
};
