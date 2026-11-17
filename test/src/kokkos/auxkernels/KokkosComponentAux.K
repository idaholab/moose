//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComponentAux.h"

registerKokkosAuxKernel("MooseTestApp", KokkosComponentAux);

InputParameters
KokkosComponentAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Sets each variable to (c + 1) * (x + 1), where c is the index of the "
                             "variable in 'variables'.");
  return params;
}

KokkosComponentAux::KokkosComponentAux(const InputParameters & parameters) : AuxKernel(parameters)
{
}
