//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralFluidProps.h"
#include "NS.h" // Variable Term Names
#include "DimensionlessFlowNumbers.h"
#include "NavierStokesMethods.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", GeneralFluidProps);

InputParameters
GeneralFluidProps::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  params.addClassDescription("Computes fluid properties using a (P, T) formulation");

  params.addRequiredCoupledVar(NS::porosity, "porosity");
  params.addRequiredRangeCheckedParam<Real>(
      "characteristic_length",
      "characteristic_length > 0.0 ",
      "characteristic length for Reynolds number calculation");
  return params;
}

GeneralFluidProps::GeneralFluidProps(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _eps(coupledValue(NS::porosity)),
    _d(getParam<Real>("characteristic_length")),

    _pressure(getADMaterialProperty<Real>(NS::pressure)),
    _T_fluid(getADMaterialProperty<Real>(NS::T_fluid)),
    _rho(getADMaterialProperty<Real>(NS::density)),
    _speed(getADMaterialProperty<Real>(NS::speed)),

    _drho_dp(declarePropertyDerivative<Real>(NS::density, NS::pressure)),
    _drho_dT(declarePropertyDerivative<Real>(NS::density, NS::T_fluid)),

    _cp(declareADProperty<Real>(NS::cp)),
    _dcp_dp(declarePropertyDerivative<Real>(NS::cp, NS::pressure)),
    _dcp_dT(declarePropertyDerivative<Real>(NS::cp, NS::T_fluid)),

    _cv(declareADProperty<Real>(NS::cv)),

    _mu(declareADProperty<Real>(NS::mu)),
    _dmu_dp(declarePropertyDerivative<Real>(NS::mu, NS::pressure)),
    _dmu_dT(declarePropertyDerivative<Real>(NS::mu, NS::T_fluid)),

    _k(declareADProperty<Real>(NS::k)),
    _dk_dp(declarePropertyDerivative<Real>(NS::k, NS::pressure)),
    _dk_dT(declarePropertyDerivative<Real>(NS::k, NS::T_fluid)),

    _Pr(declareADProperty<Real>(NS::Prandtl)),
    _dPr_dp(declarePropertyDerivative<Real>(NS::Prandtl, NS::pressure)),
    _dPr_dT(declarePropertyDerivative<Real>(NS::Prandtl, NS::T_fluid)),

    _Re(declareADProperty<Real>(NS::Reynolds)),
    _dRe_dp(declarePropertyDerivative<Real>(NS::Reynolds, NS::pressure)),
    _dRe_dT(declarePropertyDerivative<Real>(NS::Reynolds, NS::T_fluid)),

    _Re_h(declareADProperty<Real>(NS::Reynolds_hydraulic)),
    _Re_i(declareADProperty<Real>(NS::Reynolds_interstitial))
{
}

void
GeneralFluidProps::computeQpProperties()
{
  auto raw_pressure = MetaPhysicL::raw_value(_pressure[_qp]);
  auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid[_qp]);

  // Density is not a material property because we will calculate it using
  // FluidDensityAux as needed.
  Real dummy = 0;
  _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, _drho_dp[_qp], _drho_dT[_qp]);

  _cv[_qp] = _fluid.cv_from_p_T(_pressure[_qp], _T_fluid[_qp]);
  _cp[_qp] = _fluid.cp_from_p_T(_pressure[_qp], _T_fluid[_qp]);
  _mu[_qp] = _fluid.mu_from_p_T(_pressure[_qp], _T_fluid[_qp]);
  _k[_qp] = _fluid.k_from_p_T(_pressure[_qp], _T_fluid[_qp]);
  _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, _dcp_dp[_qp], _dcp_dT[_qp]);
  _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, _dmu_dp[_qp], _dmu_dT[_qp]);
  _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, _dk_dp[_qp], _dk_dT[_qp]);

  static constexpr Real small_number = 1e-8;

  _Pr[_qp] = fp::prandtl(_cp[_qp], _mu[_qp], std::max(_k[_qp], small_number));
  _dPr_dp[_qp] = NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu[_qp]),
                                               MetaPhysicL::raw_value(_cp[_qp]),
                                               MetaPhysicL::raw_value(_k[_qp]),
                                               _dmu_dp[_qp],
                                               _dcp_dp[_qp],
                                               _dk_dp[_qp]);
  _dPr_dT[_qp] = NS::prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu[_qp]),
                                               MetaPhysicL::raw_value(_cp[_qp]),
                                               MetaPhysicL::raw_value(_k[_qp]),
                                               _dmu_dT[_qp],
                                               _dcp_dT[_qp],
                                               _dk_dT[_qp]);

  // (pore / particle) Reynolds number based on superficial velocity and
  // characteristic length. Only call Reynolds() one time to compute all three so that
  // we don't redundantly check that viscosity is not too close to zero.
  _Re[_qp] = std::max(
      fp::reynolds(_rho[_qp], _eps[_qp] * _speed[_qp], _d, std::max(_mu[_qp], small_number)), 1.0);
  _dRe_dp[_qp] = NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re[_qp]),
                                                MetaPhysicL::raw_value(_rho[_qp]),
                                                MetaPhysicL::raw_value(_mu[_qp]),
                                                _drho_dp[_qp],
                                                _dmu_dp[_qp]);
  _dRe_dT[_qp] = NS::reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re[_qp]),
                                                MetaPhysicL::raw_value(_rho[_qp]),
                                                MetaPhysicL::raw_value(_mu[_qp]),
                                                _drho_dT[_qp],
                                                _dmu_dT[_qp]);

  // (hydraulic) Reynolds number
  _Re_h[_qp] = _Re[_qp] / std::max(1 - _eps[_qp], small_number);

  // (interstitial) Reynolds number
  _Re_i[_qp] = _Re[_qp] / _eps[_qp];
}
