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
 * A ReporterPointSource DiracKernel is used to create variable valued point sources.
 * Coordinates and values are given by a Reporter.  Values and coordinates for the point
 * source are allowed change as the Reporter is updated.
 */
class ReporterPointSource : public DiracKernel, public ReporterInterface
{
public:
  static InputParameters validParams();
  ReporterPointSource(const InputParameters & parameters);
  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;
  void errorCheck(const std::string & input_name, std::size_t reporterSize);

  /// values at each xyz coordinate
  const std::vector<Real> & _values;
  /// convenience vectors (these are not const because reporters can change their size)
  std::vector<Real> _ones_vec;
  std::vector<Real> _zeros_vec;
  std::vector<Point> _zeros_pts;
  /// x coordinate
  const std::vector<Real> & _coordx;
  /// y coordinate
  const std::vector<Real> & _coordy;
  ///z coordinate
  const std::vector<Real> & _coordz;
  ///xyz point
  const std::vector<Point> & _point;
  /// weights to scale value by
  const std::vector<Real> & _weight;

  /// map to associate points with their index into the vpp value
  std::map<Point, size_t> _point_to_index;
};
