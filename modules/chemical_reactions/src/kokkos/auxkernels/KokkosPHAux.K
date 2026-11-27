//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPHAux.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosPHAux);

InputParameters
KokkosPHAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("h_conc", "The molar concentration of free H+ ions in solution");
  params.addCoupledVar("activity_coeff", 1.0, "Activity coefficient of H+. Default is 1");
  params.addClassDescription("pH of solution");
  return params;
}

KokkosPHAux::KokkosPHAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _hplus(kokkosCoupledValue("h_conc")),
    _gamma(kokkosCoupledValue("activity_coeff"))
{
}
