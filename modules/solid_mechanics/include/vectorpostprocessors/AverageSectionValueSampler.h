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

  virtual void initialSetup() override;
  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Reference to the mesh
  const std::shared_ptr<MooseMesh> & _mesh;

  /// Variables to output
  std::vector<VariableName> _variables;

  /// Indices of the variables in their systems
  std::vector<unsigned int> _var_numbers;

  /// Vector of outputs, where each entry is the vector of average values for single variable at the selected points along the axis
  std::vector<VectorPostprocessorValue *> _output_vector;

  // Block ids over which this postprocessor does the computation
  std::vector<SubdomainID> _block_ids;

  /// Axis direction of the structural component
  const Point _direction;

  /// Starting or reference point of the structural component to locate nodes on the cross section
  const Point _reference_point;

  /// Axial positions along the component at which average values are computed
  std::vector<Real> _positions;

  /// Whether a symmetry plane has been defined by the user
  const bool _have_symmetry_plane;

  /// Vector normal to a symmetry plane, optionally defined if the section has a symmetry plane
  RealVectorValue _symmetry_plane;

  /// Whether to automatically locate positions along section for averaging field values
  const bool _automatically_locate_positions;

  /// Tolerance to identify nodes on the user-prescribed cross section
  const Real _tolerance;

  /// Number of nodes for computing output (local and global). We allow each section to have different
  /// numbers of nodes (this would allow local, regular refinement in the mesh)
  std::vector<unsigned int> _number_of_nodes;

  /// Tolerance to disambiguate cross section locations in different components within the same block
  const Real _cross_section_maximum_radius;

  /// Whether to require the number of nodes at each axial location to be equal
  const bool _require_equal_node_counts;

  /// Whether node locations need to be identified and nodes at positions need to be counted
  bool _need_mesh_initializations;

private:
  /**
   * Determine axial distance of the point from the component's reference point.
   * If the in-plane distance is greater than the input parameter
   * cross section maximum radius, return the maximum value for a Real.
   * @param node The node whose distance from a plane is to be considered
   */
  Real axialPosition(const Node & node) const;

  /**
   * Automatically identify all axial positions of nodes within the component
   * and store their unique values (within tolerance) in _lengths.
   */
  void automaticallyLocatePositions();
};
