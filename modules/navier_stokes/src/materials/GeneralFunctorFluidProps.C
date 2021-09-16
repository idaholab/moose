//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralFunctorFluidProps.h"
#include "NS.h" // Variable Term Names
#include "DimensionlessFlowNumbers.h"
#include "NavierStokesMethods.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", GeneralFunctorFluidProps);

InputParameters
GeneralFunctorFluidProps::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties functor userobject");
  params.addClassDescription("Creates functor fluid properties using a (P, T) formulation");

  params.addRequiredCoupledVar(NS::porosity, "porosity");
  params.addRequiredRangeCheckedParam<Real>(
      "characteristic_length",
      "characteristic_length > 0.0 ",
      "characteristic length for Reynolds number calculation");
  return params;
}

GeneralFunctorFluidProps::GeneralFunctorFluidProps(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _eps(*getVarHelper<MooseVariableFV<Real>>(NS::porosity, 0)),
    _d(getParam<Real>("characteristic_length")),

    _pressure(*getVarHelper<MooseVariableFV<Real>>(NS::pressure, 0)),
    _T_fluid(*getVarHelper<MooseVariableFV<Real>>(NS::T_fluid, 0)),

    _rho(*getVarHelper<MooseVariableFV<Real>>(NS::density, 0)),
    _speed(*getVarHelper<MooseVariableFV<Real>>(NS::speed, 0)),

    _drho_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::density, NS::pressure))),
    _drho_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::density, NS::T_fluid))),
    _drho_dt(declareFunctorProperty<Real>(NS::time_deriv(NS::density))),

    _cp(declareFunctorProperty<ADReal>(NS::cp)),
    _dcp_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::cp, NS::pressure))),
    _dcp_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::cp, NS::T_fluid))),
    _dcp_dt(declareFunctorProperty<Real>(NS::time_deriv(NS::cp))),

    _cv(declareFunctorProperty<ADReal>(NS::cv)),

    _mu(declareFunctorProperty<ADReal>(NS::mu)),
    _dmu_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::mu, NS::pressure))),
    _dmu_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::mu, NS::T_fluid))),

    _k(declareFunctorProperty<ADReal>(NS::k)),
    _dk_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::k, NS::pressure))),
    _dk_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::k, NS::T_fluid))),

    _Pr(declareFunctorProperty<ADReal>(NS::Prandtl)),
    _dPr_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::Prandtl, NS::pressure))),
    _dPr_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::Prandtl, NS::T_fluid))),

    _Re(declareFunctorProperty<ADReal>(NS::Reynolds)),
    _dRe_dp(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::Reynolds, NS::pressure))),
    _dRe_dT(declareFunctorProperty<Real>(derivativePropertyNameFirst(NS::Reynolds, NS::T_fluid))),

    _Re_h(declareFunctorProperty<ADReal>(NS::Reynolds_hydraulic)),
    _Re_i(declareFunctorProperty<ADReal>(NS::Reynolds_interstitial))
{
  _cv.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    return _fluid.cv_from_p_T(_pressure(r, t), _T_fluid(r, t));
  });
  _cp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    return _fluid.cp_from_p_T(_pressure(r, t), _T_fluid(r, t));
  });
  _mu.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    return _fluid.mu_from_p_T(_pressure(r, t), _T_fluid(r, t));
  });
  _k.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    return _fluid.k_from_p_T(_pressure(r, t), _T_fluid(r, t));
  });

  // _drho_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real drho_dp, drho_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
  //   return drho_dp;
  // });
  // _drho_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real drho_dp, drho_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.rho_from_p_T(raw_pressure, raw_T_fluid, dummy, drho_dp, drho_dT);
  //   return drho_dT;
  // });
  //
  // _dcp_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dcp_dp, dcp_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
  //   return dcp_dp;
  // });
  // _dcp_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dcp_dp, dcp_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.cp_from_p_T(raw_pressure, raw_T_fluid, dummy, dcp_dp, dcp_dT);
  //   return dcp_dT;
  // });
  //
  // _dmu_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dmu_dp, dmu_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
  //   return dmu_dp;
  // });
  // _dmu_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dmu_dp, dmu_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.mu_from_p_T(raw_pressure, raw_T_fluid, dummy, dmu_dp, dmu_dT);
  //   return dmu_dT;
  // });
  //
  // _dk_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dk_dp, dk_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
  //   return dk_dp;
  // });
  // _dk_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> Real {
  //   Real dk_dp, dk_dT, dummy;
  //   auto raw_pressure = MetaPhysicL::raw_value(_pressure(r, t));
  //   auto raw_T_fluid = MetaPhysicL::raw_value(_T_fluid(r, t));
  //
  //   _fluid.k_from_p_T(raw_pressure, raw_T_fluid, dummy, dk_dp, dk_dT);
  //   return dk_dT;
  // });
  //
  _Pr.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    static constexpr Real small_number = 1e-8;

    return fp::prandtl(_cp(r, t), _mu(r, t), std::max(_k(r, t), small_number));
  });
  // _dPr_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
  //   return prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu(r, t)),
  //                                    MetaPhysicL::raw_value(_cp(r, t)),
  //                                    MetaPhysicL::raw_value(_k(r, t)),
  //                                    _dmu_dp(r, t),
  //                                    _dcp_dp(r, t),
  //                                    _dk_dp(r, t));
  // });
  // _dPr_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
  //   return prandtlPropertyDerivative(MetaPhysicL::raw_value(_mu(r, t)),
  //                                    MetaPhysicL::raw_value(_cp(r, t)),
  //                                    MetaPhysicL::raw_value(_k(r, t)),
  //                                    _dmu_dT(r, t),
  //                                    _dcp_dT(r, t),
  //                                    _dk_dT(r, t));
  // });

  // (pore / particle) Reynolds number based on superficial velocity and
  // characteristic length. Only call Reynolds() one time to compute all three so that
  // we don't redundantly check that viscosity is not too close to zero.
  _Re.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    static constexpr Real small_number = 1e-8;

    return std::max(
          fp::reynolds(_rho(r, t),
                       _eps(r, t) * _speed(r, t),
                       _d,
                       std::max(_mu(r, t), small_number)),
                       1.0);
  });
  // _dRe_dp.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
  //   return reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re(r, t)),
  //                                     MetaPhysicL::raw_value(_rho(r, t)),
  //                                     MetaPhysicL::raw_value(_mu(r, t)),
  //                                     _drho_dp(r, t),
  //                                     _dmu_dp(r, t));
  // });
  // _dRe_dT.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
  //   return reynoldsPropertyDerivative(MetaPhysicL::raw_value(_Re(r, t)),
  //                                     MetaPhysicL::raw_value(_rho(r, t)),
  //                                     MetaPhysicL::raw_value(_mu(r, t)),
  //                                     _drho_dT(r, t),
  //                                     _dmu_dT(r, t));
  // });

  // (hydraulic) Reynolds number
  _Re_h.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    static constexpr Real small_number = 1e-8;

    return _Re(r, t) / std::max(1 - _eps(r, t), small_number);
  });

  // (interstitial) Reynolds number
  _Re_i.setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
    return _Re(r, t) / _eps(r, t);;
  });
}
