//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KineticDisPreRateAux.h"

/**
 * Calculate the kinetic mineral species concentrations according to
 * transient state theory rate law.
 */
class KineticDisPreConcAux : public KineticDisPreRateAux
{
public:
  static InputParameters validParams();

  KineticDisPreConcAux(const InputParameters & parameters);

  virtual ~KineticDisPreConcAux() {}

protected:
  virtual Real computeValue() override;

  const VariableValue & _u_old;
};
