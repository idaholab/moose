//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EulerAngleFileReader.h"
#include "ReporterInterface.h"

/**
 * Update Euler angle from reporter value
 */
class EulerAngleUpdateFromReporter : public EulerAngleFileReader
{
public:
  static InputParameters validParams();

  EulerAngleUpdateFromReporter(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual void UpdateEulerAngle();

private:
  /// Euler angles' components for every grain
  const std::vector<Real> & _euler_angle_0;
  const std::vector<Real> & _euler_angle_1;
  const std::vector<Real> & _euler_angle_2;

  /// Corresponding grain IDs - it is of Real type from reporter
  const std::vector<Real> & _grain_id;

  bool _first_time;
};
