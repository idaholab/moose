/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PHAux.h"

template <>
InputParameters
validParams<PHAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
