/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KineticDisPreConcAux.h"

template <>
InputParameters
validParams<KineticDisPreConcAux>()
{
  InputParameters params = validParams<KineticDisPreRateAux>();
  params.addClassDescription("Concentration of secondary kinetic species");
  return params;
}

KineticDisPreConcAux::KineticDisPreConcAux(const InputParameters & parameters)
  : KineticDisPreRateAux(parameters)
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
