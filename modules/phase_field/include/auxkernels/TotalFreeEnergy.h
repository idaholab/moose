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
 * Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined
 * in a material and called f_name
 */
class TotalFreeEnergy : public TotalFreeEnergyBase
{
public:
  static InputParameters validParams();

  TotalFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Bulk free energy material property
  const MaterialProperty<Real> & _F;

  /// Gradient interface free energy coefficients
  std::vector<const MaterialProperty<Real> *> _kappas;
};
