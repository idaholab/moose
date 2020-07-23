#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "SubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in csv format to be used for post-processing
 */
class exitValue : public GeneralUserObject, public Coupleable
{
public:
  exitValue(const InputParameters & params);
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;

protected:
  SubChannelMesh & _mesh;
  Eigen::MatrixXd _exitValue;
  const VariableValue & _value;
  std::string _file_name;

public:
  static InputParameters validParams();
};
