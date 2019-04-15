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
                    params.addParam<bool>("beta_p",
                                          false,
                                          "Include effective plastic work driving energy.");
                    params.addParam<bool>("beta_e", false, "Include elastic work driving energy.");
                    params.addParam<Real>("W0", 0, "plastic work threshold.");
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
    _Ee(adDeclareADProperty<RankTwoTensor>(_base_name + "elastic_strain")),
    _Ep(adDeclareADProperty<RankTwoTensor>(_base_name + "plastic_strain")),
    _cauchy_stress(adDeclareADProperty<RankTwoTensor>(_base_name + "cauchy_stress")),
    _Wp(adDeclareADProperty<Real>("Wp")),
    _Wp_old(adGetMaterialPropertyOld<Real>("Wp")),
    _W0(adGetParam<Real>("W0")),
    _beta_p(adGetParam<bool>("beta_p")),
    _beta_e(adGetParam<bool>("beta_e"))
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
  _Wp[_qp] = 0;
  _hist[_qp] = 3.0 * 10.5 / 16 / 0.4;
  //_hist[_qp] = 0;
}

template <ComputeStage compute_stage>
void
ADComputeHyperElastoPlasticPFFractureStress<compute_stage>::computeQpStress()
{
  ADReal lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  ADReal mu = _elasticity_tensor[_qp](0, 1, 0, 1);
  ADReal kappa = lambda + 2.0 * mu / LIBMESH_DIM;

  ADReal c = _c[_qp];

  // if (c < 0.0)
  //   c = 0.0;
  // else if (c > 1.0)
  //   c = 1.0;

  Real c_old = _c_old[_qp];

  // Real gp = (1.0 - c_old) * (1.0 - c_old) * (1 - _kdamage) + _kdamage;
  ADReal gp = (1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage;
  // Real gp = 1.0;
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
                   gp * std::sqrt(2.0 / 3) * (_yield_stress + _alpha_old[_qp] * _k);

  ADReal Ie_bar = 1.0 / 3 * be_trial.trace();

  ADReal plastic_increment = 0;

  if (f_trial <= 0.0)
  {
    s = s_trial;
    _alpha[_qp] = _alpha_old[_qp];
  }
  else
  {
    ADReal mu_bar = g * mu * Ie_bar;
    plastic_increment = (f_trial / 2.0 / mu_bar) / (1 + gp * _k / 3 / mu_bar);
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

  ADRankTwoTensor dev_be = s / mu / g;
  Real A = MetaPhysicL::raw_value(dev_be(0, 0));
  Real B = MetaPhysicL::raw_value(dev_be(0, 1));
  Real C = MetaPhysicL::raw_value(dev_be(0, 2));
  Real E = MetaPhysicL::raw_value(dev_be(1, 1));
  Real F = MetaPhysicL::raw_value(dev_be(1, 2));
  Real M = MetaPhysicL::raw_value(dev_be(2, 2));

  Real AA = A + E + M;
  Real BB = A * E + A * M + E * M - C * C - B * B - F * F;
  Real CC = A * E * M + 2 * B * C * F - C * C * E - A * F * F - B * B * M;

  Real I_n = MetaPhysicL::raw_value(Ie_bar);
  Real resid = I_n * I_n * I_n + AA * I_n * I_n + BB * I_n + CC - 1.0;
  int iter = 0;
  while (resid > 1.0e-12 && iter < 100)
  {
    Real jacob = 3 * I_n * I_n + 2 * I_n * AA + BB;
    Real delta_I = -resid / jacob;
    I_n = I_n + delta_I;
    resid = I_n * I_n * I_n + AA * I_n * I_n + BB * I_n + CC - 1.0;
    iter++;
  }

  if (iter == 100)
    mooseError("Solve for Ie in return mapping fails.");

  // if (_current_elem->id() == 0 && _qp == 0)
  //   std::cout << "iter = " << iter << ", I_n = " << I_n << ", resid = " << resid << std::endl;

  Ie_bar = I_n;

  // Update the intermediate configuration
  _be_bar[_qp] = s / mu / g + Ie_bar * I;

  _stress[_qp] = tau * ((_deformation_gradient[_qp].transpose()).inverse());

  ADRankTwoTensor be = _be_bar[_qp] / std::pow(J, -2.0 / 3);
  ADRankTwoTensor Cp =
      _deformation_gradient[_qp].transpose() * (be.inverse()) * _deformation_gradient[_qp];

  _Ep[_qp] = 0.5 * (Cp - I);
  _Ee[_qp] = _mechanical_strain[_qp] - _Ep[_qp];
  _cauchy_stress[_qp] = tau / J;

  // if (_current_elem->id() == 0 && _qp == 0)
  //   std::cout << "c = " << MetaPhysicL::raw_value(c) << std::endl;
  // std::cout << "yield condition = "
  //           << MetaPhysicL::raw_value(
  //                  std::sqrt((tau.deviatoric()).doubleContraction(tau.deviatoric())) -
  //                  std::sqrt(2.0 / 3) * (_yield_stress + _k * _alpha[_qp]))
  //           << std::endl;
  // std::cout << "DET(be) = " <<
  // MetaPhysicL::raw_value(_be_bar[_qp].det()) << std::endl;
  // std::cout << "f_trial = " << MetaPhysicL::raw_value(f_trial) << ", ||tau|| = "
  //           << MetaPhysicL::raw_value(
  //                  std::sqrt((tau.deviatoric()).doubleContraction(tau.deviatoric())))
  //           << ", yield stresss = " << _yield_stress + _k *
  //           << ", alpha_old =  " << MetaPhysicL::raw_value(_alpha_old[_qp]) << ", _k = " <<
  //           _k
  //           << std::endl;

  ADReal G0_pos = 0;

  ADRankTwoTensor C_bar =
      std::pow(J, -2.0 / 3) * (_deformation_gradient[_qp].transpose() * _deformation_gradient[_qp]);

  // if (J >= 1)
  //   G0_pos = 0.5 * kappa * (0.5 * (J * J - 1) - std::log(J)) +
  //            0.5 * mu * (C_bar.doubleContraction(Cp.inverse()) - 3);
  // else
  //   G0_pos = 0.5 * mu * (C_bar.doubleContraction(Cp.inverse()) - 3);

  if (_beta_e)
    if (J >= 1)
      G0_pos =
          0.5 * kappa * (0.5 * (J * J - 1) - std::log(J)) + 0.5 * mu * (_be_bar[_qp].trace() - 3);
    else
      G0_pos = 0.5 * mu * (_be_bar[_qp].trace() - 3);

  _Wp[_qp] = _Wp_old[_qp] +
             plastic_increment * std::sqrt((tau.deviatoric()).doubleContraction(tau.deviatoric()));

  if (_beta_p && _Wp[_qp] >= _W0)
    G0_pos += (_Wp[_qp] - _W0);

  // Assign history variable and derivative
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];
}
