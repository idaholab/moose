#pragma once

#include "FileOutput.h"
#include "SubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in csv format to be used for post-processing
 */
class NormalSliceValues : public FileOutput
{
public:
  NormalSliceValues(const InputParameters & params);
  virtual void output(const ExecFlagType & type) override;

protected:
  SubChannelMesh & _mesh;
  Eigen::MatrixXd _exitValue;
  const VariableName & _variable;
  const Real & _height;

public:
  static InputParameters validParams();
};
