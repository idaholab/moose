//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
TwoPhaseFluidProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();
  params.set<std::string>("fp_type") = "two-phase-fp";

  params.addParam<UserObjectName>("fp_liquid",
                                  "Liquid single-phase fluid properties user object name");
  params.addParam<UserObjectName>("fp_vapor",
                                  "Vapor single-phase fluid properties user object name");

  return params;
}

TwoPhaseFluidProperties::TwoPhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters),

    _liquid_name(isParamValid("fp_liquid") ? getParam<UserObjectName>("fp_liquid")
                                           : UserObjectName(name() + ":liquid")),
    _vapor_name(isParamValid("fp_vapor") ? getParam<UserObjectName>("fp_vapor")
                                         : UserObjectName(name() + ":vapor"))
{
  // If the single-phase fluid properties user object names are not provided, it
  // is implied that these objects will be created by a derived class. In this
  // case, we need to check that these user objects do not already exist.
  if (!isParamValid("fp_liquid"))
    if (_tid == 0 && _fe_problem.hasUserObject(_liquid_name))
      paramError("fp_liquid",
                 "The two-phase fluid properties object '" + name() + "' is ",
                 "trying to create a single-phase fluid properties object with ",
                 "name '",
                 _liquid_name,
                 "', but a single-phase fluid properties ",
                 "object with this name already exists.");
  if (!isParamValid("fp_vapor"))
    if (_tid == 0 && _fe_problem.hasUserObject(_vapor_name))
      paramError("fp_vapor",
                 "The two-phase fluid properties object '" + name() + "' is ",
                 "trying to create a single-phase fluid properties object with ",
                 "name '",
                 _vapor_name,
                 "', but a single-phase fluid properties ",
                 "object with this name already exists.");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

Real
TwoPhaseFluidProperties::T_triple() const
{
  mooseError(__PRETTY_FUNCTION__, " is not implemented.");
}

ADReal
TwoPhaseFluidProperties::T_sat(const ADReal & p) const
{
  const Real p_real = p.value();
  const Real T_sat_real = T_sat(p_real);
  const Real dT_sat_dp_real = dT_sat_dp(p_real);

  ADReal T_sat = T_sat_real;
  T_sat.derivatives() = p.derivatives() * dT_sat_dp_real;
  return T_sat;
}

ADReal
TwoPhaseFluidProperties::p_sat(const ADReal & T) const
{
  const Real T_real = T.value();
  const Real p_sat_real = p_sat(T_real);
  const Real dp_sat_dT_real = 1.0 / dT_sat_dp(p_sat_real);

  ADReal p_sat = p_sat_real;
  p_sat.derivatives() = T.derivatives() * dp_sat_dT_real;
  return p_sat;
}

Real
TwoPhaseFluidProperties::h_lat(Real p, Real T) const
{
  return _fp_vapor->h_from_p_T(p, T) - _fp_liquid->h_from_p_T(p, T);
}

ADReal
TwoPhaseFluidProperties::h_lat(const ADReal & p, const ADReal & T) const
{
  return _fp_vapor->h_from_p_T(p, T) - _fp_liquid->h_from_p_T(p, T);
}

Real
TwoPhaseFluidProperties::L_fusion() const
{
  mooseError(__PRETTY_FUNCTION__, " is not implemented.");
}

Real TwoPhaseFluidProperties::sigma_from_T(Real /*T*/) const
{
  mooseError(__PRETTY_FUNCTION__, " is not implemented.");
}

ADReal
TwoPhaseFluidProperties::sigma_from_T(const ADReal & T) const
{
  const Real T_real = T.value();
  const Real sigma_real = sigma_from_T(T_real);
  const Real dsigma_dT = dsigma_dT_from_T(T_real);

  ADReal sigma = sigma_real;
  sigma.derivatives() = T.derivatives() * dsigma_dT;
  return sigma;
}

Real TwoPhaseFluidProperties::dsigma_dT_from_T(Real /*T*/) const
{
  mooseError(__PRETTY_FUNCTION__, " is not implemented.");
}

#pragma GCC diagnostic pop
