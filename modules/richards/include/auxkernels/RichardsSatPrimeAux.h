//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RICHARDSSATPRIMEAUX_H
#define RICHARDSSATPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSat.h"

// Forward Declarations
class RichardsSatPrimeAux;

template <>
InputParameters validParams<RichardsSatPrimeAux>();

/**
 * Derivative of fluid Saturation wrt effective saturation
 */
class RichardsSatPrimeAux : public AuxKernel
{
public:
  RichardsSatPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// User object defining saturation as a function of effective saturation
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATPRIMEAUX_H
