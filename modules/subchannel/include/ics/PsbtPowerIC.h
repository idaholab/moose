#pragma once

#include "PsbtIC.h"
#include <Eigen/Dense>

class PsbtPowerIC;

template <>
InputParameters validParams<PsbtPowerIC>();

/**
 * Sets the linear heat rate for the PSBT 01-6232 fluid temperature benchmark.
 */
class PsbtPowerIC : public PsbtIC
{
public:
  PsbtPowerIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;
  Real power;
  int numberoflines;
  std::string filename;
  Eigen::MatrixXd power_dis;

private:
  Eigen::MatrixXd _ref_qprime;
};
