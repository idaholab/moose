//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RICHARDSSEFFAUX_H
#define RICHARDSSEFFAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

// Forward Declarations
class RichardsSeffAux;

template <>
InputParameters validParams<RichardsSeffAux>();

/**
 * Calculates effective saturation for a specified variable
 */
class RichardsSeffAux : public AuxKernel
{
public:
  RichardsSeffAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /**
   * The user object that defines effective saturation
   * as function of porepressure (or porepressures in the
   * multiphase situation)
   */
  const RichardsSeff & _seff_UO;

  /**
   * the porepressure values (this will be length N
   * where N is the number of arguments that the _seff_UO requires)
   * Eg, for twophase simulations N=2
   */
  std::vector<const VariableValue *> _pressure_vals;
};

#endif // RICHARDSSEFFAUX_H
