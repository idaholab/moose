//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "libmesh/communicator.h"

class SectionDisplacementAverage : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  SectionDisplacementAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// References to the displaced mesh
  const std::shared_ptr<MooseMesh> & _displaced_mesh;

  /// References to the mesh mesh
  const std::shared_ptr<MooseMesh> & _mesh;

  /// Output with d-dimensional cross sectional displacement x
  VectorPostprocessorValue & _section_displacements_x;

  /// Output with d-dimensional cross sectional displacement y
  VectorPostprocessorValue & _section_displacements_y;

  /// Output with d-dimensional cross sectional displacement z
  VectorPostprocessorValue & _section_displacements_z;

  // Block ids over which this postprocessor does the computation
  std::vector<SubdomainID> _block_ids;

  /// Axis direction of the structural component
  const Point _direction;

  /// Starting or reference point of the structural component to locate nodes on the cross section
  const Point _reference_point;

  /// Locations for of the cross section
  const std::vector<Real> _lengths;

  /// Tolerance to identify nodes on the user-prescribed cross section
  const Real _tolerance;

  /// Number of nodes for computing output (local and global). We allow each section to have different
  /// numbers of nodes (this would allow local, regular refinement in the structural component mesh)
  std::vector<unsigned int> _number_of_nodes;

private:
  /// Axis direction of the structural component
  Real distancePointPlane(const Node & node,
                          const Point & axis_direction,
                          const Point & reference_point,
                          const Real length) const;
};
