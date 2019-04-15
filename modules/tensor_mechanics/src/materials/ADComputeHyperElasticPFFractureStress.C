//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeHyperElasticPFFractureStress.h"

registerADMooseObject("TensorMechanicsApp", ADComputeHyperElasticPFFractureStress);

defineADValidParams(ADComputeHyperElasticPFFractureStress,
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
ADComputeHyperElasticPFFractureStress<compute_stage>::ADComputeHyperElasticPFFractureStress(
    const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adGetADMaterialProperty<RankFourTensor>(_elasticity_tensor_name)),
    _c(adCoupledValue("c")),
    _c_old(coupledValueOld("c")),
    _kdamage(adGetParam<Real>("kdamage")),
    _use_current_hist(adGetParam<bool>("use_current_history_variable")),
    _hist(adDeclareADProperty<Real>("hist")),
    _hist_old(adGetMaterialPropertyOld<Real>("hist")),
    _cauchy_stress(adDeclareADProperty<RankTwoTensor>(_base_name + "cauchy_stress"))
{
}

template <ComputeStage compute_stage>
void
ADComputeHyperElasticPFFractureStress<compute_stage>::initQpStatefulProperties()
{
  ADComputeStressBase<compute_stage>::initQpStatefulProperties();
  //_hist[_qp] = 3.0 * 138 / 16 / 14.2;
  _hist[_qp] = 0.0;
}

template <ComputeStage compute_stage>
void
ADComputeHyperElasticPFFractureStress<compute_stage>::computeQpStress()
{
  ADReal lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  ADReal mu = _elasticity_tensor[_qp](0, 1, 0, 1);
  ADReal k = lambda + 2.0 * mu / LIBMESH_DIM;

  const ADReal c = _c[_qp];

  ADReal J = _deformation_gradient[_qp].det();
  ADRankTwoTensor I;
  I.zero();
  I.addIa(1.0);

  ADRankTwoTensor C = _deformation_gradient[_qp].transpose() * _deformation_gradient[_qp];

  if (_current_elem->id() == 0 && _qp == 0)
  {
    std::cout << "c = " << MetaPhysicL::raw_value(c) << ", J = " << MetaPhysicL::raw_value(J)
              << std::endl;
  }

  ADRankTwoTensor stress_pos, stress_neg;
  ADReal G0_pos;

  if (J >= 1)
  {
    stress_pos = 0.5 * k * (J * J - 1) * (C.inverse()) +
                 mu * std::pow(J, -2.0 / 3) * (I - 1.0 / 3 * C.trace() * C.inverse());
    stress_neg.zero();
    G0_pos = 0.5 * k * (0.5 * (J * J - 1) - std::log(J)) +
             0.5 * mu * (std::pow(J, -2.0 / 3) * C.trace() - 3);
  }
  else
  {
    stress_pos = mu * std::pow(J, -2.0 / 3) * (I - 1.0 / 3 * C.trace() * C.inverse());
    stress_neg = 0.5 * k * (J * J - 1) * (C.inverse());
    G0_pos = 0.5 * mu * (std::pow(J, -2.0 / 3) * C.trace() - 3);
  }

  // Assign history variable and derivative
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];

  // Damage associated with positive component of stress
  ADRankTwoTensor PK2 =
      stress_pos * ((1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage) + stress_neg;
  _stress[_qp] = _deformation_gradient[_qp] * PK2;

  _cauchy_stress[_qp] =
      1.0 / J * _deformation_gradient[_qp] * PK2 * _deformation_gradient[_qp].transpose();
}
