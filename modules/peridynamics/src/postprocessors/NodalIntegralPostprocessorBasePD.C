//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalIntegralPostprocessorBasePD.h"

InputParameters
NodalIntegralPostprocessorBasePD::validParams()
{
  InputParameters params = NodalPostprocessorBasePD::validParams();
  params.addClassDescription("Base class for peridynamic nodal integral Postprocessors");

  return params;
}

NodalIntegralPostprocessorBasePD::NodalIntegralPostprocessorBasePD(
    const InputParameters & parameters)
  : NodalPostprocessorBasePD(parameters), _integral_value(0)
{
}

void
NodalIntegralPostprocessorBasePD::initialize()
{
  _integral_value = 0;
}

void
NodalIntegralPostprocessorBasePD::execute()
{
  _integral_value += computeNodalValue() * _pdmesh.getNodeVolume(_current_node->id());
}

Real
NodalIntegralPostprocessorBasePD::getValue()
{
  return _integral_value;
}
void
NodalIntegralPostprocessorBasePD::finalize()
{
  gatherSum(_integral_value);
}
void
NodalIntegralPostprocessorBasePD::threadJoin(const UserObject & uo)
{
  const NodalIntegralPostprocessorBasePD & pps =
      static_cast<const NodalIntegralPostprocessorBasePD &>(uo);
  _integral_value += pps._integral_value;
}
