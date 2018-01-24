/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KINETICDISPRECONCAUX_H
#define KINETICDISPRECONCAUX_H

#include "KineticDisPreRateAux.h"

class KineticDisPreConcAux;

template <>
InputParameters validParams<KineticDisPreConcAux>();

/**
 * Calculate the kinetic mineral species concentrations according to
 * transient state theory rate law.
 */
class KineticDisPreConcAux : public KineticDisPreRateAux
{
public:
  KineticDisPreConcAux(const InputParameters & parameters);

  virtual ~KineticDisPreConcAux() {}

protected:
  virtual Real computeValue() override;
};

#endif // KINETICDISPRECONCAUX_H
