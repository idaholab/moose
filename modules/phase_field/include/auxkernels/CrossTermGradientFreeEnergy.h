//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalFreeEnergyBase.h"

// Forward Declarations

/**
 * Cross term gradient free energy contribution used by ACMultiInterface
 */
class CrossTermGradientFreeEnergy : public TotalFreeEnergyBase
{
public:
  static InputParameters validParams();

  CrossTermGradientFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  std::vector<std::vector<const MaterialProperty<Real> *>> _kappas;
};
