//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KineticDisPreConcAux.h"

registerMooseObject("ChemicalReactionsApp", KineticDisPreConcAux);

InputParameters
KineticDisPreConcAux::validParams()
{
  InputParameters params = KineticDisPreRateAux::validParams();
  params.addClassDescription("Concentration of secondary kinetic species");
  return params;
}

KineticDisPreConcAux::KineticDisPreConcAux(const InputParameters & parameters)
  : KineticDisPreRateAux(parameters), _u_old(uOld())
{
}

Real
KineticDisPreConcAux::computeValue()
{
  const Real kinetic_rate = KineticDisPreRateAux::computeValue();

  Real u_new_aux = _u_old[_qp] + kinetic_rate * _dt;

  // Bound concentration for the dissolution case
  if (u_new_aux < 0.0)
    u_new_aux = 0.0;

  return u_new_aux;
}
