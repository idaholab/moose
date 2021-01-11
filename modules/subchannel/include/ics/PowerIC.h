#pragma once

#include "QuadSubChannelBaseIC.h"
#include <Eigen/Dense>

/**
 * Sets the axial heat rate for each pin according to a radial power distribution
 * and a user defined axial power shape.
 */
class PowerIC : public QuadSubChannelBaseIC
{
public:
  PowerIC(const InputParameters & params);
  Real value(const Point & p) override;
  virtual void initialSetup() override;

protected:
  Real _power;
  unsigned int _numberoflines;
  std::string _filename;
  Eigen::MatrixXd _power_dis;
  const Function & _axial_heat_rate;

private:
  Eigen::MatrixXd _ref_qprime; /// average linear heat rate over the whole pin in W/m
  Eigen::MatrixXd _ref_power;  /// actual pin power in W
  Eigen::MatrixXd
      _pin_power_correction; /// its the correction that will be applied to the estimated calculation [unitless]
  Eigen::MatrixXd
      _estimate_power; /// its a matrix which will hold the total estimated power of each pin [W]

public:
  static InputParameters validParams();
};
