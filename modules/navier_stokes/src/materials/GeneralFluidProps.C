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
#include "InputErrorChecking.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", GeneralFluidProps);

defineADValidParams(
    GeneralFluidProps,
    ADMaterial,
    params.addRequiredParam<UserObjectName>(nms::fluid, "Fluid userobject");
    params.addClassDescription("Computes fluid properties using (P, T) formulation");

    params.addRequiredCoupledVar(nms::porosity, "porosity");
    params.addRangeCheckedParam<Real>(nms::pebble_diameter,
                                              nms::pebble_diameter + " > 0.0 ",
                                              "Pebble diameter");
    params.addRangeCheckedParam<Real>("characteristic_length",
                                              "characteristic_length > 0.0 ",
                                              "characteristic length for Reynolds number calculation"););

GeneralFluidProps::GeneralFluidProps(const InputParameters & parameters)
  : DerivativeMaterialInterface<ADMaterial>(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(nms::fluid)),
    _eps(coupledValue(nms::porosity)),

    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _rhoE(getADMaterialProperty<Real>(nms::total_energy_density)),
    _T_fluid(getADMaterialProperty<Real>(nms::T_fluid)),
    _rho(getADMaterialProperty<Real>(nms::density)),
    _speed(getADMaterialProperty<Real>(nms::speed)),

    _drho_dp(declarePropertyDerivative<Real>(nms::density, nms::pressure)),
    _drho_dT(declarePropertyDerivative<Real>(nms::density, nms::T_fluid)),

    _cp(declareADProperty<Real>(nms::cp)),
    _dcp_dp(declarePropertyDerivative<Real>(nms::cp, nms::pressure)),
    _dcp_dT(declarePropertyDerivative<Real>(nms::cp, nms::T_fluid)),

    _cv(declareADProperty<Real>(nms::cv)),

    _mu(declareADProperty<Real>(nms::mu)),
    _dmu_dp(declarePropertyDerivative<Real>(nms::mu, nms::pressure)),
    _dmu_dT(declarePropertyDerivative<Real>(nms::mu, nms::T_fluid)),

    _k(declareADProperty<Real>(nms::k)),
    _dk_dp(declarePropertyDerivative<Real>(nms::k, nms::pressure)),
    _dk_dT(declarePropertyDerivative<Real>(nms::k, nms::T_fluid)),

    _Pr(declareADProperty<Real>(nms::Prandtl)),
    _dPr_dp(declarePropertyDerivative<Real>(nms::Prandtl, nms::pressure)),
    _dPr_dT(declarePropertyDerivative<Real>(nms::Prandtl, nms::T_fluid)),

    _Re(declareADProperty<Real>(nms::Reynolds)),
    _dRe_dp(declarePropertyDerivative<Real>(nms::Reynolds, nms::pressure)),
    _dRe_dT(declarePropertyDerivative<Real>(nms::Reynolds, nms::T_fluid)),

    _Re_h(declareADProperty<Real>(nms::Reynolds_hydraulic)),
    _Re_i(declareADProperty<Real>(nms::Reynolds_interstitial))
{
  if (parameters.isParamSetByUser("characteristic_length"))
  {
    _d = getParam<Real>("characteristic_length");
    checkUnusedInputParameter(parameters, nms::pebble_diameter, "When specifying a characteristic length");
  }
  else
  {
    if (parameters.isParamSetByUser(nms::pebble_diameter))
      _d = getParam<Real>(nms::pebble_diameter);
    else
      errorMessage(parameters, "A 'characteristic_length' must be specified for computing "
        "the Reynolds number. For pebble bed systems, you can specify '" + nms::pebble_diameter + "'.");
  }
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

  _Pr[_qp] = fp::prandtl(_cp[_qp], _mu[_qp], std::max(_k[_qp], NS_DEFAULT_VALUES::epsilon));
  _dPr_dp[_qp] = prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu[_qp]),
                                           MetaPhysicL::raw_value(_cp[_qp]),
                                           MetaPhysicL::raw_value(_k[_qp]),
                                           _dmu_dp[_qp],
                                           _dcp_dp[_qp],
                                           _dk_dp[_qp]);
  _dPr_dT[_qp] = prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu[_qp]),
                                           MetaPhysicL::raw_value(_cp[_qp]),
                                           MetaPhysicL::raw_value(_k[_qp]),
                                           _dmu_dT[_qp],
                                           _dcp_dT[_qp],
                                           _dk_dT[_qp]);

  // (pore / particle) Reynolds number based on superficial velocity and
  // pebble diameter. Only call Reynolds() one time to compute all three so that
  // we don't redundantly check that viscosity is not too close to zero.
  _Re[_qp] = std::max(
      fp::reynolds(
          _rho[_qp], _eps[_qp] * _speed[_qp], _d, std::max(_mu[_qp], NS_DEFAULT_VALUES::epsilon)),
      1.0);
  _dRe_dp[_qp] = reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re[_qp]),
                                            MetaPhysicL::raw_value(_rho[_qp]),
                                            MetaPhysicL::raw_value(_mu[_qp]),
                                            _drho_dp[_qp],
                                            _dmu_dp[_qp]);
  _dRe_dT[_qp] = reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re[_qp]),
                                            MetaPhysicL::raw_value(_rho[_qp]),
                                            MetaPhysicL::raw_value(_mu[_qp]),
                                            _drho_dT[_qp],
                                            _dmu_dT[_qp]);

  // (hydraulic) Reynolds number
  _Re_h[_qp] = _Re[_qp] / std::max(1 - _eps[_qp], NS_DEFAULT_VALUES::epsilon);

  // (interstitial) Reynolds number
  _Re_i[_qp] = _Re[_qp] / _eps[_qp];
}
