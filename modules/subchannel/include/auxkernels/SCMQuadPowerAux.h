//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "SubChannelMesh.h"

class QuadSubChannelMesh;

/**
 * Sets the axial heat rate for each pin according to a radial power distribution
 * and a user defined axial power shape.
 */
class SCMQuadPowerAux : public AuxKernel
{
public:
  SCMQuadPowerAux(const InputParameters & params);
  virtual Real computeValue() override;
  virtual void initialSetup() override;
  virtual void timestepSetup() override;

protected:
  const QuadSubChannelMesh & _quadMesh;
  /// The total power of the assembly
  const PostprocessorValue & _power;
  /// The number of lines associated with the radial power profile .txt file
  unsigned int _numberoflines;
  /// The name of the radial power profile file
  std::string _filename;
  /// Matrix that holds the relative pin power
  Eigen::MatrixXd _power_dis;
  const Function & _axial_heat_rate;
  /// Whether to print the per-pin power correction every time it is computed
  const bool _verbose;
  /// Trapezoidal integral of the axial power shape over each pin on the SCM axial grid [m]
  Eigen::MatrixXd _shape_integral;

  /// Compute the trapezoidal integral of the axial power shape over each pin
  void computeShapeIntegral();

public:
  static InputParameters validParams();
};
