//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialAverageAux.h"

registerMooseObject("MooseTestApp", RadialAverageAux);

InputParameters
RadialAverageAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Auxkernel to output averaged material from RadialAverage");
  params.addRequiredParam<UserObjectName>("average_UO", "Radial Average user object");
  return params;
}

RadialAverageAux::RadialAverageAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _average(this->template getUserObject<RadialAverage>("average_UO").getAverage())
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
RadialAverageAux::computeValue()
{
  _elem_avg = _average.find(_current_elem->id());
  if (_elem_avg != _average.end())
    return _elem_avg->second[_qp];
  return 0.0;
}
