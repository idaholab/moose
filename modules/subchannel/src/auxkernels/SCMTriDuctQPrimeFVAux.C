//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriDuctQPrimeFVAux.h"

registerMooseObject("SubChannelApp", SCMTriDuctQPrimeFVAux);
registerMooseObjectRenamed("SubChannelApp",
                           TriDuctQPrimeFVAux,
                           "06/30/2025 24:00",
                           SCMTriDuctQPrimeFVAux);

InputParameters
SCMTriDuctQPrimeFVAux::validParams()
{
  InputParameters params = DiffusionFluxFVAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>(
      "flat_to_flat", "distance from one flat side of the duct to the opposite flat side [m]");
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
