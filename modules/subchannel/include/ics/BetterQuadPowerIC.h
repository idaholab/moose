#pragma once

#include "BetterQuadSubChannelBaseIC.h"
#include <Eigen/Dense>

/**
 * Sets the axial heat rate for each pin according to a radial power distribution
 * and a user defined axial power shape.
 */
class BetterQuadPowerIC : public BetterQuadSubChannelBaseIC
{
public:
  BetterQuadPowerIC(const InputParameters & params);
  Real value(const Point & p) override;
  virtual void initialSetup() override;

protected:
  Real _power;
  unsigned int _numberoflines;
  std::string _filename;
  Eigen::MatrixXd _power_dis;
  const Function & _axial_heat_rate;
  /// Average linear heat rate over the whole pin [W/m]
  Eigen::MatrixXd _ref_qprime;
  /// Actual pin power [W]
  Eigen::MatrixXd _ref_power;
  /// The correction that will be applied to the estimated calculation [unitless]
  Eigen::MatrixXd _pin_power_correction;
  /// Matrix which will hold the total estimated power of each pin [W]
  Eigen::MatrixXd _estimate_power;

public:
  static InputParameters validParams();
};
