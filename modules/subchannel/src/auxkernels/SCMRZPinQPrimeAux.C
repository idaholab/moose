//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMRZPinQPrimeAux.h"

registerMooseObject("SubChannelApp", SCMRZPinQPrimeAux);
registerMooseObjectRenamed("SubChannelApp", RZPinQPrimeAux, "06/30/2025 24:00", SCMRZPinQPrimeAux);

InputParameters
SCMRZPinQPrimeAux ::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription(
      "Axial heat rate on pin surface for a 2D-RZ axi-symmetric fuel pin model");
  return params;
}

SCMRZPinQPrimeAux ::SCMRZPinQPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters)
{
  for (auto & b : blockIDs())
  {
    if (_subproblem.getCoordSystem(b) != Moose::COORD_RZ)
      mooseError(this->name(), ": This kernel must be calculated on an -RZ Mesh");
  }
}

Real
SCMRZPinQPrimeAux ::computeValue()
{
  return DiffusionFluxAux::computeValue() * M_PI * 2.0 * abs(_q_point[_qp](0));
}
