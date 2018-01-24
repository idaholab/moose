//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
