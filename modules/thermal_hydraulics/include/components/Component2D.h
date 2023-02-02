//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMeshComponent.h"

class Component2D : public GeneratedMeshComponent
{
public:
  Component2D(const InputParameters & params);

  virtual void buildMesh() override;

  /**
   * Gets the total width of all transverse regions
   */
  const Real & getTotalWidth() const { return _total_width; }

  /**
   * Gets the number of transverse regions
   */
  unsigned int getNumHS() const { return _number_of_hs; }

  /**
   * Returns true if there is a transverse region of a given name
   *
   * @param name   The name of the transverse region
   */
  bool hasBlock(const std::string & name) const;

  /**
   * Gets the names of the transverse regions
   */
  const std::vector<std::string> & getNames() const { return _names; }

  /**
   * Gets the volumes of the transverse regions
   */
  const std::vector<Real> & getVolumes() const { return _volume; }

  /**
   * Returns true if the supplied boundary is in the given vector
   */
  bool isBoundaryInVector(const BoundaryName & boundary_name,
                          const std::vector<BoundaryName> & boundary_name_vector) const;

  /**
   * Returns true if this component has the supplied boundary
   */
  bool hasBoundary(const BoundaryName & boundary_name) const;

  /**
   * Gets boundary info associated with the component boundary
   *
   * @param[in] boundary  Boundary name of a component boundary
   *
   * @return The list of tuples (element id, local side id) that is associated with boundary
   * `boundary`
   */
  const std::vector<std::tuple<dof_id_type, unsigned short int>> &
  getBoundaryInfo(const BoundaryName & boundary_name) const;

  /**
   * Gets the axial offset for the mesh
   */
  Real getAxialOffset() const { return _axial_offset; }

protected:
  virtual void check() const override;

  /**
   * Builds a 2D, first-order mesh
   */
  void build2DMesh();

  /**
   * Builds a 2D, second-order mesh
   */
  void build2DMesh2ndOrder();

  /// Number of transverse regions
  unsigned int _number_of_hs;
  /// Names of each transverse region
  std::vector<std::string> _names;
  /// Width of each transverse region
  std::vector<Real> _width;
  /// Total width of all transverse regions
  Real _total_width;
  /// Volume of each transverse region
  std::vector<Real> _volume;
  /// Number of elements in each transverse region
  std::vector<unsigned int> _n_part_elems;
  /// Total number of transverse elements
  unsigned int _total_elem_number;

  /// BC ID of the component (outer)
  std::vector<unsigned int> _outer_bc_id;
  /// BC ID of the component (inner)
  std::vector<unsigned int> _inner_bc_id;
  /// BC ID of the component (start)
  std::vector<unsigned int> _start_bc_id;
  /// BC ID of the component (end)
  std::vector<unsigned int> _end_bc_id;
  /// BC ID of the interior axial boundaries (per radial section) of the component
  std::vector<unsigned int> _interior_axial_per_radial_section_bc_id;
  /// BC ID of the axial regions of the outer boundary of the component
  std::vector<unsigned int> _axial_outer_bc_id;
  /// BC ID of the axial regions of the inner boundary of the component
  std::vector<unsigned int> _axial_inner_bc_id;
  /// BC ID of the radial regions of the start boundary of the component
  std::vector<unsigned int> _radial_start_bc_id;
  /// BC ID of the radial regions of the end boundary of the component
  std::vector<unsigned int> _radial_end_bc_id;
  /// BC ID of the inner radial boundary regions of the component
  std::vector<unsigned int> _inner_radial_bc_id;

  /// Boundary names of the outer side of the component
  std::vector<BoundaryName> _boundary_names_outer;
  /// Boundary names of the inner side of the component
  std::vector<BoundaryName> _boundary_names_inner;
  /// Boundary names of the start side of the component
  std::vector<BoundaryName> _boundary_names_start;
  /// Boundary names of the end side of the component
  std::vector<BoundaryName> _boundary_names_end;
  /// Boundary names of the interior axial boundaries (per radial section) of the component
  std::vector<BoundaryName> _boundary_names_interior_axial_per_radial_section;
  /// Boundary names of the axial regions of the outer side of the component
  std::vector<BoundaryName> _boundary_names_axial_outer;
  /// Boundary names of the axial regions of the inner side of the component
  std::vector<BoundaryName> _boundary_names_axial_inner;
  /// Boundary names of the radial regions of the start side of the component
  std::vector<BoundaryName> _boundary_names_radial_start;
  /// Boundary names of the radial regions of the end side of the component
  std::vector<BoundaryName> _boundary_names_radial_end;
  /// Boundary names of the inner radial boundary regions of the component
  std::vector<BoundaryName> _boundary_names_inner_radial;

  /// Nodes on the side of the "block"
  std::map<std::string, std::vector<unsigned int>> _side_heat_node_ids;
  /// Nodes at the outer side of the generated component
  std::vector<unsigned int> _outer_heat_node_ids;
  /// Nodes at the inner side of the generated component
  std::vector<unsigned int> _inner_heat_node_ids;

  /// Map of boundary name to list of tuples of element and side IDs for that boundary
  std::map<BoundaryName, std::vector<std::tuple<dof_id_type, unsigned short int>>>
      _hs_boundary_info;

  /// Distance by which to offset the mesh from the component axis
  mutable Real _axial_offset;

public:
  static InputParameters validParams();
};
