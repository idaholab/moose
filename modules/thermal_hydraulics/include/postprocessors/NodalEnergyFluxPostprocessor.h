//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPostprocessor.h"

/**
 * Computes sum of energy flux for a phase over nodes
 */
class NodalEnergyFluxPostprocessor : public NodalPostprocessor
{
public:
  NodalEnergyFluxPostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

protected:
  Real _value;
  const VariableValue & _arhouA;
  const VariableValue & _H;

public:
  static InputParameters validParams();
};
