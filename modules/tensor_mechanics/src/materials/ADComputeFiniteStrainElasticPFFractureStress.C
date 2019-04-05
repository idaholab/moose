//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteStrainElasticPFFractureStress.h"

registerADMooseObject("TensorMechanicsApp", ADComputeFiniteStrainElasticPFFractureStress);

defineADValidParams(ADComputeFiniteStrainElasticPFFractureStress,
                    ADComputeStressBase,
                    params.addClassDescription(
                        "Computes the stress and free energy derivatives for the phase field "
                        "fracture model, with linear isotropic elasticity");
                    params.addRequiredCoupledVar("c", "Order parameter for damage");
                    params.addParam<Real>("kdamage", 0.0, "Stiffness of damaged matrix");
                    params.addParam<bool>("use_current_history_variable",
                                          false,
                                          "Use the current value of the history variable."););

template <ComputeStage compute_stage>
ADComputeFiniteStrainElasticPFFractureStress<
    compute_stage>::ADComputeFiniteStrainElasticPFFractureStress(const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adGetADMaterialProperty<RankFourTensor>(_elasticity_tensor_name)),
    _c(adCoupledValue("c")),
    _c_old(coupledValueOld("c")),
    _kdamage(adGetParam<Real>("kdamage")),
    _use_current_hist(adGetParam<bool>("use_current_history_variable")),
    _hist(adDeclareADProperty<Real>("hist")),
    _hist_old(adGetMaterialPropertyOld<Real>("hist"))
{
}

template <ComputeStage compute_stage>
void
ADComputeFiniteStrainElasticPFFractureStress<compute_stage>::computeQpStress()
{
  ADReal lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  ADReal mu = _elasticity_tensor[_qp](0, 1, 0, 1);

  const ADReal c = _c[_qp];

  ADRankTwoTensor stress = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  ADRankTwoTensor D, Q, D_pos, D_neg;

  _mechanical_strain[_qp].diagonalize(Q, D);

  ADRankTwoTensor strain_pos, strain_neg;

  for (unsigned int i = 0; i < 3; ++i)
    D_pos(i, i) = D(i, i) > 0.0 ? D(i, i) : 0;

  D_neg = D - D_pos;

  strain_pos = Q * D_pos * Q.transpose();
  strain_neg = Q * D_neg * Q.transpose();

  ADReal strain_trace_pos =
      _mechanical_strain[_qp].trace() > 0 ? _mechanical_strain[_qp].trace() : 0;

  // Compute the positive and negative elastic energies
  ADReal G0_pos = 0.5 * lambda * strain_trace_pos * strain_trace_pos +
                  mu * strain_pos.doubleContraction(strain_pos);

  ADRankTwoTensor I;
  I.zero();
  I.addIa(1.0);

  ADRankTwoTensor stress_pos = lambda * strain_trace_pos * I + 2.0 * mu * strain_pos;

  ADRankTwoTensor stress_neg = (_elasticity_tensor[_qp] * _mechanical_strain[_qp]) - stress_pos;

  // Assign history variable and derivative
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];

  // Damage associated with positive component of stress
  _stress[_qp] = _deformation_gradient[_qp] *
                 (stress_pos * ((1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage) + stress_neg);
}
