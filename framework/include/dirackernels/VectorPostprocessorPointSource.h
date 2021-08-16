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

// Forward Declarations
class VectorPostprocessorPointSource;

template <>
InputParameters validParams<VectorPostprocessorPointSource>();

/**
 * A VectorPostprocessorPointSource DiracKernel is used to create variable valued point sources.
 * Coordinates and values are given by a vector Postprocessor.  Values and coordinates for the point
 * source are allowed change as the vector Postprocessor is updated.
 */
class VectorPostprocessorPointSource : public DiracKernel, public ReporterInterface
{
public:
  static InputParameters validParams();
  VectorPostprocessorPointSource(const InputParameters & parameters);
  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

private:
  /// values at each xyz coordinate
  const std::vector<Real> & _vpp_values;
  /// x coordinate
  const std::vector<Real> & _x_coord;
  /// y coordinate
  const std::vector<Real> & _y_coord;
  ///z coordinate
  const std::vector<Real> & _z_coord;
  /// map to associate points with their index into the vpp value
  std::map<Point, size_t> _point_to_index;
  /// helper function to read in data
  const std::vector<Real> & getPointDataHelper(const std::string & param);
};
