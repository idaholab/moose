//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMRZPinQPrimeAux.h"

registerMooseObject("MooseApp", SCMRZPinQPrimeAux);

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
}

Real
SCMRZPinQPrimeAux ::computeValue()
{
  return DiffusionFluxAux::computeValue() * M_PI * 2.0 * abs(_q_point[_qp](0));
}
