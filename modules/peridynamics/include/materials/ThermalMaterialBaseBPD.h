//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PeridynamicsMaterialBase.h"

class Function;

/**
 * Base material class for bond based peridynamic heat conduction models
 */
class ThermalMaterialBaseBPD : public PeridynamicsMaterialBase
{
public:
  static InputParameters validParams();

  ThermalMaterialBaseBPD(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  /**
   * Function to compute micro-conductivity
   * @param ave_thermal_conductivity average thermal conductivity for the current element
   */
  virtual void computePeridynamicsParams(const Real ave_thermal_conductivity) = 0;

  ///@{ Temperature variable and values
  MooseVariable * _temp_var;
  std::vector<Real> _temp;
  ///@}

  ///@{ Material properties to be stored
  MaterialProperty<Real> & _bond_heat_flow;
  MaterialProperty<Real> & _bond_dQdT;
  ///@}

  /// Thermal conductivity
  const MaterialProperty<Real> & _thermal_conductivity;

  /// Micro-conductivity
  Real _Kij;
};
