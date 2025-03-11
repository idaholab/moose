//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriDuctQPrimeAux.h"

registerMooseObject("SubChannelApp", SCMTriDuctQPrimeAux);
registerMooseObjectRenamed("SubChannelApp",
                           TriDuctQPrimeAux,
                           "06/30/2025 24:00",
                           SCMTriDuctQPrimeAux);

InputParameters
SCMTriDuctQPrimeAux::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  params.addRequiredParam<Real>(
      "flat_to_flat", "distance from one flat side of the duct to the opposite flat side [m]");
  return params;
}

SCMTriDuctQPrimeAux::SCMTriDuctQPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters), _flat_to_flat(getParam<Real>("flat_to_flat"))
{
}

Real
SCMTriDuctQPrimeAux::computeValue()
{
  return DiffusionFluxAux::computeValue() * 6 * _flat_to_flat / std::sqrt(3);
}
