//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriDuctQPrimeFVAux.h"

registerMooseObject("MooseApp", SCMTriDuctQPrimeFVAux);

InputParameters
SCMTriDuctQPrimeFVAux::validParams()
{
  InputParameters params = DiffusionFluxFVAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>("flat_to_flat", "[m]");
  return params;
}

SCMTriDuctQPrimeFVAux::SCMTriDuctQPrimeFVAux(const InputParameters & parameters)
  : DiffusionFluxFVAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
SCMTriDuctQPrimeFVAux::computeValue()
{
  return DiffusionFluxFVAux::computeValue() * 6 * _flat_to_flat / std::sqrt(3);
}
