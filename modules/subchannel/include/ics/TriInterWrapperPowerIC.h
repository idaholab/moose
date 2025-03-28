//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TriInterWrapperBaseIC.h"

class TriInterWrapperMesh;

/**
 * Sets the axial heat rate for each pin according to a radial power distribution and a user defined
 * axial power shape for hexagonal fuel assemblies.
 */
class TriInterWrapperPowerIC : public TriInterWrapperBaseIC
{
public:
  TriInterWrapperPowerIC(const InputParameters & params);
  Real value(const Point & p) override;

  virtual void initialSetup() override;

protected:
  /// total power of the fuel assembly
  Real _power;
  /// number of lines
  unsigned int _numberoflines;
  /// pin power distribution file name
  std::string _filename;
  /// pin power distribution from the input file given in "_filename"
  std::vector<Real> _power_dis;
  /// normalized axial power distribution
  const Function & _axial_heat_rate;

private:
  /// average linear heat rate over the whole pin in W/m
  std::vector<Real> _ref_qprime;
  /// actual pin power in W
  std::vector<Real> _ref_power;
  /// its the correction that will be applied to the estimated calculation [unitless]
  std::vector<Real> _pin_power_correction;
  /// its a vector which will hold the total estimated power of each pin [W]
  std::vector<Real> _estimate_power;

public:
  static InputParameters validParams();
};
