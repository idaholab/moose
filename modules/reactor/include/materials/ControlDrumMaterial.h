//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * This material evaluate a material property on elements on a rotating control drum
 * from material properties of all drum segments based on which segment the elements
 * are located.
 */
class ControlDrumMaterial : public Material
{
public:
  static InputParameters validParams();

  ControlDrumMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Rotation center
  const std::vector<Point> _rotation_centers;

  /// x/y/z/-x/-y/-z
  const MooseEnum _rotation_axis;

  /// Whether the rotation axis points to the positive direction, i.e. equal to x, y or z
  const bool _plus;

  /// Rotation offsets of all drums
  const std::vector<Real> _rotation_offsets;

  /// Segment angles (all rotation centers share the same segment angles)
  const std::vector<Real> _segment_angles;

  /// Number of rod segments
  const unsigned int _n_segments;

  /// The property this material evaluates
  MaterialProperty<Real> & _drum_property;

  /// Rotation direction (x/y/z - 0/1/2)
  unsigned int _dir;

  /// Rotation angle functors of all drums
  std::vector<const Moose::Functor<Real> *> _rotation_functors;

  /// Material properties of all segments
  std::vector<const MaterialProperty<Real> *> _segment_properties;
};
