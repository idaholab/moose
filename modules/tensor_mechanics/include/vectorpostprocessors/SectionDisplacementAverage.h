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

  /// Output with d-dimensional cross sectional displacement
  VectorPostprocessorValue & _section_displacements;

  // Block ids over which this postprocessor does the computation
  std::vector<SubdomainID> _block_ids;

  /// Axis direction of the structural component
  const Point _direction;

  /// Location for of the cross section
  const Real _length;

  /// Tolerance to identify nodes on the user-prescribed cross section
  const Real _tolerance;

  /// Number of nodes for computing output (local and global)
  unsigned int _number_of_nodes;

private:
  /// Axis direction of the structural component
  Real distancePointPlane(const Node & node, const Point & axis_direction, const Real length) const;
};
