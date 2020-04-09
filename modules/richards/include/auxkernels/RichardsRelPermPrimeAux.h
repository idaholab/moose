//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

// Forward Declarations

/**
 * Derivative of relative Permeability wrt effective saturation
 */
class RichardsRelPermPrimeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RichardsRelPermPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};
