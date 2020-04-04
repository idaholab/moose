//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultipleInelasticCosseratStress.h"

registerMooseObject("TensorMechanicsApp", ComputeMultipleInelasticCosseratStress);

InputParameters
ComputeMultipleInelasticCosseratStress::validParams()
{
  InputParameters params = ComputeMultipleInelasticStress::validParams();
  params.addClassDescription("Compute state (stress and other quantities such as plastic "
                             "strains and internal parameters) using an iterative process, as well "
                             "as Cosserat versions of these quantities.  Only elasticity is "
                             "currently implemented for the Cosserat versions."
                             "Combinations of creep models and plastic models may be used");
  return params;
}

ComputeMultipleInelasticCosseratStress::ComputeMultipleInelasticCosseratStress(
    const InputParameters & parameters)
  : ComputeMultipleInelasticStress(parameters),
    _curvature(getMaterialProperty<RankTwoTensor>("curvature")),
    _elastic_flexural_rigidity_tensor(
        getMaterialProperty<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _couple_stress(declareProperty<RankTwoTensor>("couple_stress")),
    _couple_stress_old(getMaterialPropertyOld<RankTwoTensor>("couple_stress")),
    _Jacobian_mult_couple(declareProperty<RankFourTensor>("couple_Jacobian_mult")),
    _compliance(getMaterialProperty<RankFourTensor>(_base_name + "compliance_tensor"))
{
}

void
ComputeMultipleInelasticCosseratStress::initQpStatefulProperties()
{
  ComputeMultipleInelasticStress::initQpStatefulProperties();
  _couple_stress[_qp].zero();
}

void
ComputeMultipleInelasticCosseratStress::computeQpStress()
{
  ComputeMultipleInelasticStress::computeQpStress();

  _couple_stress[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];
  if (_fe_problem.currentlyComputingJacobian())
    _Jacobian_mult_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp];

  if (_perform_finite_strain_rotations)
  {
    _couple_stress[_qp] =
        _rotation_increment[_qp] * _couple_stress[_qp] * _rotation_increment[_qp].transpose();
    _Jacobian_mult_couple[_qp].rotate(_rotation_increment[_qp]);
  }
}

void
ComputeMultipleInelasticCosseratStress::computeQpJacobianMult()
{
  if (_tangent_operator_type == TangentOperatorEnum::elastic)
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  else
  {
    _Jacobian_mult[_qp] = _consistent_tangent_operator[0];
    for (unsigned i_rmm = 1; i_rmm < _num_models; ++i_rmm)
      _Jacobian_mult[_qp] =
          _consistent_tangent_operator[i_rmm] * _compliance[_qp] * _Jacobian_mult[_qp];
  }
}

void
ComputeMultipleInelasticCosseratStress::computeAdmissibleState(
    unsigned model_number,
    RankTwoTensor & elastic_strain_increment,
    RankTwoTensor & inelastic_strain_increment,
    RankFourTensor & consistent_tangent_operator)
{
  const RankTwoTensor trial_stress = _stress[_qp];
  const RankTwoTensor applied_strain_increment = elastic_strain_increment;

  ComputeMultipleInelasticStress::computeAdmissibleState(model_number,
                                                         elastic_strain_increment,
                                                         inelastic_strain_increment,
                                                         consistent_tangent_operator);

  inelastic_strain_increment = _compliance[_qp] * (trial_stress - _stress[_qp]);
  elastic_strain_increment = applied_strain_increment - inelastic_strain_increment;
}
