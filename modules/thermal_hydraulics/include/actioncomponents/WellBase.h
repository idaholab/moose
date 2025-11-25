//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMActionComponent.h"

/**
 * Base class for injection and production wells.
 */
class WellBase : public THMActionComponent
{
public:
  static InputParameters validParams();
  WellBase(const InputParameters & params);

protected:
  virtual void addClosures() override;

  /// Adds the components common to both injection and production
  void addWellBaseComponents(bool is_production);
  /// Adds a flow channel
  void addFlowChannel(unsigned int i, bool is_production);
  /// Adds a wall at the end of a well
  void addWall(bool is_production);
  /// Adds a junction
  void addJunction(unsigned int i, bool is_production);
  /// Adds a junction coupling
  void addJunctionFlux(unsigned int i);

  /// Name of a flow channel
  std::string flowChannelName(unsigned int i) const;
  /// Name of a volume junction
  std::string volumeJunctionName(unsigned int i) const;

  /// Surface point
  const Point & _surface_point;
  /// Junction points
  const std::vector<Point> & _junction_points;
  /// Number of elements in each flow channel
  const std::vector<unsigned int> & _section_n_elems;
  /// Number of flow channels
  const unsigned int _n_sections;
  /// Coupled flow area of each junction
  const std::vector<Real> & _junction_coupling_areas;
  /// Closures name
  const std::string _closures_name;

  /// Surface point, junction points, and optional end point
  std::vector<Point> _all_points;
};
