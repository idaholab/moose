//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"
#include "ReporterInterface.h"

/**
 * Marks all elements as inside/empty if they do/don't contain a point
 */
class ReporterPointMarker : public Marker, public ReporterInterface
{
public:
  static InputParameters validParams();
  ReporterPointMarker(const InputParameters & parameters);
  virtual void markerSetup() override;

protected:
  virtual MarkerValue computeElementMarker() override;

  /// marker value to give elements containing a point
  const MarkerValue _inside;
  /// marker for elements not containing points
  const MarkerValue _empty;
  /// x coordinate
  const std::vector<Real> & _x_coord;
  /// y coordinate
  const std::vector<Real> & _y_coord;
  ///z coordinate
  const std::vector<Real> & _z_coord;
  /// Pointer to PointLocatorBase object
  std::unique_ptr<libMesh::PointLocatorBase> _pl;
  /// list of sort uniqued elements containing points
  std::set<dof_id_type> _point_elems;
};
