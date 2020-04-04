//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PHAux.h"

registerMooseObject("ChemicalReactionsApp", PHAux);

InputParameters
PHAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("h_conc", "The molar concentration of free H+ ions in solution");
  params.addCoupledVar("activity_coeff", 1.0, "Activity coefficient of H+. Default is 1");
  params.addClassDescription("pH of solution");
  return params;
}

PHAux::PHAux(const InputParameters & parameters)
  : AuxKernel(parameters), _hplus(coupledValue("h_conc")), _gamma(coupledValue("activity_coeff"))
{
}

Real
PHAux::computeValue()
{
  mooseAssert(_gamma[_qp] * _hplus[_qp] > 0.0, "Negative activity in pH is not possible");
  return -std::log10(_gamma[_qp] * _hplus[_qp]);
}
