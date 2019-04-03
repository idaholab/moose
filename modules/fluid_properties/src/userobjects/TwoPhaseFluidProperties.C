//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<TwoPhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

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

Real
TwoPhaseFluidProperties::h_lat(Real p, Real T) const
{
  return _fp_vapor->h_from_p_T(p, T) - _fp_liquid->h_from_p_T(p, T);
}

Real TwoPhaseFluidProperties::sigma_from_T(Real /*T*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}

Real TwoPhaseFluidProperties::dsigma_dT_from_T(Real /*T*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
