/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
                            unsigned /*qp*/,
                            const SymmElasticityTensor & elasticity_tensor,
                            const SymmTensor & stress_old,
                            SymmTensor & strain_increment,
                            SymmTensor & stress_new)
{
  stress_new = elasticity_tensor * strain_increment;
  stress_new += stress_old;
}
