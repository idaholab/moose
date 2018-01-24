//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticModel.h"

#include "SymmElasticityTensor.h"

template <>
InputParameters
validParams<ElasticModel>()
{
  InputParameters params = validParams<ConstitutiveModel>();
  return params;
}

ElasticModel::ElasticModel(const InputParameters & parameters) : ConstitutiveModel(parameters) {}

////////////////////////////////////////////////////////////////////////

ElasticModel::~ElasticModel() {}

////////////////////////////////////////////////////////////////////////

void
ElasticModel::computeStress(const Elem & /*current_elem*/,
                            const SymmElasticityTensor & elasticity_tensor,
                            const SymmTensor & stress_old,
                            SymmTensor & strain_increment,
                            SymmTensor & stress_new)
{
  stress_new = elasticity_tensor * strain_increment;
  stress_new += stress_old;
}
