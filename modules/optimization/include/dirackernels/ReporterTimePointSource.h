//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"
#include "ReporterInterface.h"

/**
 * Apply a point load defined by vectors.
 */
class ReporterTimePointSource : public DiracKernel, public ReporterInterface
{
public:
  static InputParameters validParams();
  ReporterTimePointSource(const InputParameters & parameters);
  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

private:
  /// x-coordinates from reporter
  const std::vector<Real> & _coordx;
  /// y-coordinates from reporter
  const std::vector<Real> & _coordy;
  /// z-coordinates from reporter
  const std::vector<Real> & _coordz;
  /// time-coordinates from reporter
  const std::vector<Real> & _coordt;
  /// values from reporter
  const std::vector<Real> & _values;
  /// The final time when we want to reverse the time index in function evaluation
  const Real & _reverse_time_end;
  /// map to associate points with their index into the vpp value
  std::map<Point, size_t> _point_to_index;

  const std::vector<Real> _empty_vec = {};
};
