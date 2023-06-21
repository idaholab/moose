/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
