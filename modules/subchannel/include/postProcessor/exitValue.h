#pragma once

#include "SubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in csv format to be used for post-processing
 */
class exitValue // need to inherit from somewhere or not??
{
public:
  exitValue(const InputParameters & params);
  void print_exitValue();

protected:
  SubChannelMesh & _mesh;
  Eigen::MatrixXd _exitValue;
  std::string _valueName;

public:
  static InputParameters validParams();
};
