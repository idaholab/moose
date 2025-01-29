//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileOutput.h"
#include "QuadSubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected variable value in matrix format at a specific plane
 */
class QuadSubChannelNormalSliceValues : public FileOutput
{
public:
  QuadSubChannelNormalSliceValues(const InputParameters & params);
  virtual void output() override;

protected:
  const QuadSubChannelMesh & _mesh;
  /// matrix that holds the value of the variable the user wants to print
  Eigen::MatrixXd _exit_value;
  /// The name of the variable
  const VariableName & _variable;
  /// The axial location where the plane is
  const Real & _height;

public:
  static InputParameters validParams();
};
