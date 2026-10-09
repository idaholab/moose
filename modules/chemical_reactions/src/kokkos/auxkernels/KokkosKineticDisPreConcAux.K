//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKineticDisPreConcAux.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosKineticDisPreConcAux);

InputParameters
KokkosKineticDisPreConcAux::validParams()
{
  InputParameters params = KokkosKineticDisPreRateAux::validParams();
  params.addClassDescription("Concentration of secondary kinetic species");
  return params;
}

KokkosKineticDisPreConcAux::KokkosKineticDisPreConcAux(const InputParameters & parameters)
  : KokkosKineticDisPreRateAux(parameters), _u_old(uOld())
{
}
