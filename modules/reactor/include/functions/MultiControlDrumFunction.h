//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "MeshMetaDataInterface.h"
#include "ElementIDInterface.h"

/**
 * A function that returns an absorber fraction for multiple control drums application.
 */
class MultiControlDrumFunction : public Function,
                                 public MeshMetaDataInterface,
                                 public ElementIDInterface
{
public:
  static InputParameters validParams();

  MultiControlDrumFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Name of the mesh generator to get MeshMetaData from
  const MeshGeneratorName _mesh_generator;
  /// Vector of angular speeds of control drums
  const std::vector<Real> _angular_speeds;
  /// Vector of initial starting angles of control drums
  const std::vector<Real> _start_angles;
  /// Vector of angular ranges of control drums
  const std::vector<Real> _angle_ranges;
  /// Start time of control drums rotation
  const Real _rotation_start_time;
  /// End time of control drums rotation
  const Real _rotation_end_time;
  /// Whether extra element id user_control_drum_id is used.
  const bool _use_control_drum_id;
  /// ExtraElementID: control drum ExtraElementID
  const dof_id_type & _control_drum_id;
  /// MeshMetaData: positions of control drums
  const std::vector<Point> & _control_drum_positions;
  /// MeshMetaData: vector of azimuthal angles of all nodes of each control drum
  const std::vector<std::vector<Real>> & _control_drums_azimuthal_meta;
};
