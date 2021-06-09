#pragma once

#include "FileOutput.h"
#include "BetterQuadSubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in matrix format to be used for post-processing
 */
class BetterNormalSliceValues : public FileOutput
{
public:
  BetterNormalSliceValues(const InputParameters & params);
  virtual void output(const ExecFlagType & type) override;

protected:
  BetterQuadSubChannelMesh & _mesh;
  Eigen::MatrixXd _exit_value;
  const VariableName & _variable;
  const Real & _height;

public:
  static InputParameters validParams();
};
