#pragma once

#include "FileOutput.h"
#include "QuadSubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in one line csv format to be used for post-processing
 */
class QuadSubChannelPointValues : public FileOutput
{
public:
  QuadSubChannelPointValues(const InputParameters & params);
  virtual void output(const ExecFlagType & type) override;
  //  virtual std::string filename() override;
protected:
  QuadSubChannelMesh & _mesh;
  Eigen::MatrixXd _exit_value;
  const VariableName & _variable;
  const Real & _height;
  const int & _nx;
  const int & _ny;

public:
  static InputParameters validParams();
};
