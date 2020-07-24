#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "SubChannelMesh.h"
#include <Eigen/Dense>

/**
 * Prints out a user selected value in csv format to be used for post-processing
 */
class NormalSliceValuesCSV : public GeneralUserObject, public Coupleable
{
public:
  NormalSliceValuesCSV(const InputParameters & params);
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;

protected:
  SubChannelMesh & _mesh;
  Eigen::MatrixXd _exitValue;
  const VariableValue & _value;
  std::string _file_name;
  const Real & _height;

public:
  static InputParameters validParams();
};
