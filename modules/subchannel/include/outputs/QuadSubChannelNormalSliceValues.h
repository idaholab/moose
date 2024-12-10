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
 * Prints out a user selected value in matrix format to be used for post-processing
 */
class QuadSubChannelNormalSliceValues : public FileOutput
{
public:
  QuadSubChannelNormalSliceValues(const InputParameters & params);
  virtual void output() override;

protected:
  QuadSubChannelMesh & _mesh;
  Eigen::MatrixXd _exit_value;
  const VariableName & _variable;
  const Real & _height;

public:
  static InputParameters validParams();
};
