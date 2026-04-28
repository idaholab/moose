//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedAux.h"

registerKokkosAuxKernel("MooseApp", KokkosParsedAux);

InputParameters
KokkosParsedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += ParsedObjectBase::validParams();
  params.addClassDescription(
      "Sets a field variable value to the evaluation of a parsed expression.");
  return params;
}

KokkosParsedAux::KokkosParsedAux(const InputParameters & parameters)
  : AuxKernel(parameters), ParsedObjectBase(this)
{
  initParsed(this, coupledNames("coupled_variables"));
}
