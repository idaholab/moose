//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeHyperElastoPlasticPFFractureStress.h"
#include "metaphysicl/numberarray.h"
#include "metaphysicl/dualnumber.h"

registerADMooseObject("TensorMechanicsApp", ADComputeHyperElastoPlasticPFFractureStress);

defineADValidParams(ADComputeHyperElastoPlasticPFFractureStress,
                    ADComputeStressBase,
                    params.addClassDescription(
                        "Computes the stress and free energy derivatives for the phase field "
                        "fracture model, with linear isotropic elasticity");
                    params.addRequiredCoupledVar("c", "Order parameter for damage");
                    params.addParam<Real>("kdamage", 0.0, "Stiffness of damaged matrix");
                    params.addParam<bool>("use_current_history_variable",
                                          false,
                                          "Use the current value of the history variable.");
                    params.addRequiredParam<Real>("yield_stress", "Yield stress.");
                    params.addParam<Real>("linear_hardening_coefficient",
                                          0,
                                          "Linear hardening coefficient (Default value is '0' "
                                          "which indicates perfect plasticity.)."));

template <ComputeStage compute_stage>
ADComputeHyperElastoPlasticPFFractureStress<
    compute_stage>::ADComputeHyperElastoPlasticPFFractureStress(const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adGetADMaterialProperty<RankFourTensor>(_elasticity_tensor_name)),
    _c(adCoupledValue("c")),
    _c_old(coupledValueOld("c")),
    _kdamage(adGetParam<Real>("kdamage")),
    _use_current_hist(adGetParam<bool>("use_current_history_variable")),
    _hist(adDeclareADProperty<Real>("hist")),
    _hist_old(adGetMaterialPropertyOld<Real>("hist")),
    _be_bar(adDeclareADProperty<RankTwoTensor>(_base_name + "volume_perserving_be")),
    _alpha(adDeclareADProperty<Real>(_base_name + "alpha")),
    _deformation_gradient_old(
        adGetMaterialPropertyOldByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _be_bar_old(adGetMaterialPropertyOldByName<RankTwoTensor>(_base_name + "volume_perserving_be")),
    _alpha_old(adGetMaterialPropertyOldByName<Real>(_base_name + "alpha")),
    _yield_stress(adGetParam<Real>("yield_stress")),
    _k(adGetParam<Real>("linear_hardening_coefficient")),
    _be(adDeclareADProperty<RankTwoTensor>(_base_name + "elastic_left_cauchy_green_strain")),
    _Cp(adDeclareADProperty<RankTwoTensor>(_base_name + "plastic_right_cauchy_green_strain"))
{
}

template <ComputeStage compute_stage>
void
ADComputeHyperElastoPlasticPFFractureStress<compute_stage>::initQpStatefulProperties()
{
  ADComputeStressBase<compute_stage>::initQpStatefulProperties();
  _be_bar[_qp].zero();
  _be_bar[_qp].addIa(1.0);
  _alpha[_qp] = 0;
}

template <ComputeStage compute_stage>
void
ADComputeHyperElastoPlasticPFFractureStress<compute_stage>::computeQpStress()
{
  ADReal lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  ADReal mu = _elasticity_tensor[_qp](0, 1, 0, 1);
  ADReal kappa = lambda + 2.0 * mu / LIBMESH_DIM;

  const ADReal c = _c[_qp];
  Real c_old = _c_old[_qp];

  Real gp_old = (1.0 - c_old) * (1.0 - c_old) * (1 - _kdamage) + _kdamage;
  ADReal g = (1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage;

  ADRankTwoTensor I;
  I.zero();
  I.addIa(1.0);

  // Update the current configuration
  ADRankTwoTensor incremental_F =
      _deformation_gradient[_qp] * (_deformation_gradient_old[_qp].inverse());

  // Compute the damage/elastic predictor
  ADRankTwoTensor incremental_F_bar = std::pow(incremental_F.det(), -1.0 / 3) * incremental_F;
  ADRankTwoTensor be_trial = incremental_F_bar * _be_bar_old[_qp] * (incremental_F_bar.transpose());

  ADRankTwoTensor s_trial = g * mu * (be_trial.deviatoric());

  ADRankTwoTensor s;

  // Check for plastic loading
  ADReal f_trial = std::sqrt(s_trial.doubleContraction(s_trial)) -
                   gp_old * std::sqrt(2.0 / 3) * (_yield_stress + _alpha_old[_qp] * _k);

  ADReal Ie_bar = 1.0 / 3 * be_trial.trace();

  if (f_trial <= 0.0)
  {
    s = s_trial;
    _alpha[_qp] = _alpha_old[_qp];
  }
  else
  {
    ADReal mu_bar = g * mu * Ie_bar;
    ADReal plastic_increment = (f_trial / 2.0 / mu_bar) / (1 + gp_old * _k / 3 * mu_bar);
    s = s_trial - 2 * mu_bar * plastic_increment *
                      (s_trial / (std::sqrt(s_trial.doubleContraction(s_trial))));
    _alpha[_qp] = _alpha_old[_qp] + std::sqrt(2.0 / 3) * plastic_increment;
  }

  // Update the Kirchhoff stress
  ADReal J = _deformation_gradient[_qp].det();
  ADReal p;
  if (J >= 1)
    p = g * 0.5 * kappa * (J - 1.0 / J);
  else
    p = 0.5 * kappa * (J - 1.0 / J);

  ADRankTwoTensor tau = J * p * I + s;

  // std::cout << "be_trial" << std::endl;
  // be_trial.print();
  // ADRankTwoTensor FFT = _deformation_gradient[_qp] * (_deformation_gradient[_qp].transpose());
  // std::cout << "FFT" << std::endl;
  // FFT.print();
  // std::cout << "J = " << MetaPhysicL::raw_value(J) << std::endl;
  // std::cout << "FACTOR = " << MetaPhysicL::raw_value(std::pow(incremental_F.det(), -1.0 / 3))
  //           << std::endl;

  // Update the intermediate configuration
  _be_bar[_qp] = s / mu + Ie_bar * I;

  _stress[_qp] = tau * (_deformation_gradient[_qp].transpose()).inverse();

  _be[_qp] = _be_bar[_qp] / std::pow(J, -2.0 / 3);
  _Cp[_qp] =
      _deformation_gradient[_qp].transpose() * (_be[_qp].inverse()) * _deformation_gradient[_qp];

  ADReal G0_pos = 0;

  // Assign history variable and derivative
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];
}
