//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearElasticStress.h"

registerMooseObject("TensorMechanicsApp", ComputeLinearElasticStress);

InputParameters
ComputeLinearElasticStress::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

ComputeLinearElasticStress::ComputeLinearElasticStress(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ComputeLinearElasticStress::initialSetup()
{
  // _base_name + "unstabilized_deformation_gradient" is only declared if we're
  // using the Lagrangian kernels.  It's okay to invoke this small strain
  // material if you are using that kernel system and the
  // ComputeLagrangianWrappedStress wrapper
  if (hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment") &&
      !hasBlockMaterialProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient"))
    mooseError("This linear elastic stress calculation only works for small strains; use "
               "ComputeFiniteStrainElasticStress for simulations using incremental and finite "
               "strains.");
}

void
ComputeLinearElasticStress::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
