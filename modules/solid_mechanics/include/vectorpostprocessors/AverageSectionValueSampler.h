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

class AverageSectionValueSampler : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  AverageSectionValueSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Reference to the displaced mesh
  const std::shared_ptr<MooseMesh> & _displaced_mesh;

  /// Reference to the mesh
  const std::shared_ptr<MooseMesh> & _mesh;

  /// Variables to output
  std::vector<VariableName> _variables;

  /// Vector of outputs, where each entry is the vector of average values for single variable at the selected points along the axis
  std::vector<VectorPostprocessorValue *> _output_vector;

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
  /// numbers of nodes (this would allow local, regular refinement in the mesh)
  std::vector<unsigned int> _number_of_nodes;

  /// Tolerance to disambiguate cross section locations in different components within the same block
  const Real _cross_section_maximum_radius;

private:
  /**
   * Determine the distance of a point from a plane at a specified axial distance from the
   * component's reference point. If the in-plane distance is greater than the input parameter
   * cross section maximum radius, return a large number.
   * @param node The node whose distance from a plane is to be considered
   * @param reference_point Reference point for the component
   * @param length Axial position on the component to be considered
   */
  Real
  distancePointToPlane(const Node & node, const Point & reference_point, const Real length) const;
};
