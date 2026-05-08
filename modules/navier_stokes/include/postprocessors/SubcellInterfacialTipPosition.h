//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

#include <vector>

class MooseMesh;

/**
 * Reconstructs the extremal interface point from neighboring FV cell values by linearly
 * interpolating the threshold crossing between cell centroids. The reported value can be any
 * coordinate component of that extremal tip point.
 */
class SubcellInterfacialTipPosition : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SubcellInterfacialTipPosition(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

private:
  void updateCandidate(const Point & point);
  bool pointIsWithinTipBand(const Point & point) const;

  const MooseMesh & _mesh;
  const Moose::Functor<Real> & _volume_fraction;
  const unsigned int _tip_direction;
  const unsigned int _reported_component;
  const bool _search_max;
  const bool _search_reported_max;
  const Real _threshold;
  const Real _minimum_alignment;
  const Real _tip_band_width;
  const Real _value_if_no_interface;

  Point _tip_point;
  Real _reported_value;
  bool _found_candidate;
  std::vector<Point> _crossing_points;
};
