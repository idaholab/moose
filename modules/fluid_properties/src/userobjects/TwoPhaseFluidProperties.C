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
  return params;
}

TwoPhaseFluidProperties::TwoPhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
  _liquid_name = name() + ":liquid";
  _vapor_name = name() + ":vapor";

  // check that the user has not already created these single-phase properties
  if (_fe_problem.hasUserObject(_liquid_name))
    mooseError("The two-phase fluid properties object '" + name() + "' is ",
               "trying to create a single-phase fluid properties object with ",
               "name '",
               _liquid_name,
               "', but a single-phase fluid properties ",
               "object with this name already exists.");
  if (_fe_problem.hasUserObject(_vapor_name))
    mooseError("The two-phase fluid properties object '" + name() + "' is ",
               "trying to create a single-phase fluid properties object with ",
               "name '",
               _vapor_name,
               "', but a single-phase fluid properties ",
               "object with this name already exists.");
}

const UserObjectName &
TwoPhaseFluidProperties::getLiquidName() const
{
  return _liquid_name;
}

const UserObjectName &
TwoPhaseFluidProperties::getVaporName() const
{
  return _vapor_name;
}

Real
TwoPhaseFluidProperties::h_lat(Real p, Real T) const
{
  return _fp_vapor->h(p, T) - _fp_liquid->h(p, T);
}
