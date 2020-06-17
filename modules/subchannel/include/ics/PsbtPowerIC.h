#pragma once

#include "PsbtIC.h"
#include <Eigen/Dense>

/**
 * This class Sets the linear heat rate for the PSBT mixing fluid temperature benchmark.
  Thus far there is only a radial distribution and not an axial one.
 */
class PsbtPowerIC : public PsbtIC
{
public:
  PsbtPowerIC(const InputParameters & params);

  /**
  * This function is used to calculate the axial heat rate per subchannel cell
  */
  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;
  Real _power;
  int _numberoflines;
  std::string _filename;
  Eigen::MatrixXd _power_dis;

private:
  Eigen::MatrixXd _ref_qprime;

public:
  static InputParameters validParams();
};
