//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TriSubChannelBaseIC.h"

class TriSubChannelMesh;

/**
 * Sets the axial heat rate for each pin according to a radial power distribution and a user defined
 * axial power shape for hexagonal fuel assemblies.
 */
class SCMTriPowerIC : public TriSubChannelBaseIC
{
public:
  SCMTriPowerIC(const InputParameters & params);
  Real value(const Point & p) override;

  virtual void initialSetup() override;

protected:
  Real _power;
  unsigned int _numberoflines;
  std::string _filename;
  Eigen::MatrixXd _power_dis;
  const Function & _axial_heat_rate;
  /// Average linear heat rate over the whole pin [W/m]
  Eigen::MatrixXd _ref_qprime;
  /// Actual pin power [W]
  Eigen::MatrixXd _ref_power;
  /// The correction that will be applied to the estimated calculation [unitless]
  Eigen::MatrixXd _pin_power_correction;
  /// Matrix which will hold the total estimated power of each pin [W]
  Eigen::MatrixXd _estimate_power;

public:
  static InputParameters validParams();
};
