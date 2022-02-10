//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 *
 */
class SpecificInternalEnergyIC : public InitialCondition
{
public:
  SpecificInternalEnergyIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhoE;

public:
  static InputParameters validParams();
};
