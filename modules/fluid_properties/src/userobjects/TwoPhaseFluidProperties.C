/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
