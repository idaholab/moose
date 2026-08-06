//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://github.com/idaholab/moose/blob/master/LICENSE

#include "SCMFrictionPressureDrop.h"
#include "SubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", SCMFrictionPressureDrop);

InputParameters
SCMFrictionPressureDrop::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Reports the surface average pressure loss across the whole subchannel assembly due to axial "
      "wall friction and local form losses.");
  return params;
}

SCMFrictionPressureDrop::SCMFrictionPressureDrop(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _scm_problem(dynamic_cast<const SubChannel1PhaseProblem *>(&_fe_problem)),
    _value(0)
{
  if (!_scm_problem)
    mooseError(name(), " can only be used with a SubChannel1PhaseProblem.");
}

void
SCMFrictionPressureDrop::execute()
{
  _value = _scm_problem->getFrictionPressureDrop();
}

Real
SCMFrictionPressureDrop::getValue() const
{
  return _value;
}
