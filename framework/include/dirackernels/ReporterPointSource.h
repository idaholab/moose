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
  /**
   * Add points and check for duplicate points.  Values and weights are
   * combined for duplicated points
   * @param point The point to add
   * @param id index for dirac kernel being added
   */
  void fillPoint(const Point & point, const dof_id_type id);
  void errorCheck(const std::string & input_name, std::size_t reporterSize);

  /// bool if duplicate points values and weights should be combined
  const bool _combine_duplicates;
  /// bool if data format read in is points
  const bool _read_in_points;
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

  /// map from an added point to it's weighted value
  std::unordered_map<Point, Real> _point_to_weightedValue;
};
