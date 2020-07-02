#pragma once

#include "IC.h"
#include <Eigen/Dense>

/**
 * Sets the linear heat rate for the PSBT mixing fluid temperature benchmark.
  Thus far there is only a radial distribution and not an axial one i.e : constant axial linear heat flux
  value is the method in InitialConditions that returns the linear heat flux per subchannel cell
 */
class PowerIC : public IC
{
public:
  PowerIC(const InputParameters & params);
  Real value(const Point & p) override;

protected:
  SubChannelMesh & _mesh;
  Real _power;
  int _numberoflines;
  std::string _filename;
  Eigen::MatrixXd _power_dis;
  const Function & _axial_heat_rate;

private:
  Eigen::MatrixXd _ref_qprime;
  Eigen::MatrixXd _pin_power_correction; //unitless

public:
  static InputParameters validParams();
};
