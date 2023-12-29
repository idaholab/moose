//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeLinearElasticStress.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

registerMooseObject("TensorMechanicsApp", ADComputeLinearElasticStress);
registerMooseObject("TensorMechanicsApp", ADSymmetricLinearElasticStress);

template <typename R2, typename R4>
InputParameters
ADComputeLinearElasticStressTempl<R2, R4>::validParams()
{
  InputParameters params = ADComputeStressBaseTempl<R2>::validParams();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

template <typename R2, typename R4>
ADComputeLinearElasticStressTempl<R2, R4>::ADComputeLinearElasticStressTempl(
    const InputParameters & parameters)
  : ADComputeStressBaseTempl<R2>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(this->template getADMaterialProperty<R4>(_elasticity_tensor_name))
{
}

template <typename R2, typename R4>
void
ADComputeLinearElasticStressTempl<R2, R4>::initialSetup()
{
  if (this->template hasBlockMaterialProperty<R2>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains; use "
               "ADComputeFiniteStrainElasticStress for simulations using incremental and finite "
               "strains.");
}

template <typename R2, typename R4>
void
ADComputeLinearElasticStressTempl<R2, R4>::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}

template class ADComputeLinearElasticStressTempl<RankTwoTensor, RankFourTensor>;
template class ADComputeLinearElasticStressTempl<SymmetricRankTwoTensor, SymmetricRankFourTensor>;
