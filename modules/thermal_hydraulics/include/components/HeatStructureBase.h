//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricalComponent.h"
#include "HeatConductionModel.h"

/// heat structure side
enum class HeatStructureSideType
{
  INNER = 0,
  OUTER = 1,
  START = 2,
  END = 3
};

class HeatStructureBase : public GeometricalComponent
{
public:
  HeatStructureBase(const InputParameters & params);

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  const std::vector<unsigned int> & getSideNodeIds(const std::string & name) const;
  const std::vector<unsigned int> & getOuterNodeIds() const;
  const std::vector<BoundaryName> & getOuterBoundaryNames() const;
  const std::vector<BoundaryName> & getInnerBoundaryNames() const;
  const std::vector<BoundaryName> & getStartBoundaryNames() const;
  const std::vector<BoundaryName> & getEndBoundaryNames() const;

  const Real & getTotalWidth() const { return _total_width; }
  unsigned int getNumHS() const { return _number_of_hs; }

  /**
   * Check that the block with name 'name' is defined in the heat structure
   * @param name The name to be looked for
   * @return true if the heat structure has a block with name 'name', otherwise false
   */
  bool hasBlock(const std::string & name) const;

  /**
   * Return the names of heat structure blocks
   *
   * @return The vector of heat structure block names
   */
  const std::vector<std::string> & getNames() const { return _names; }

  /**
   * Get index of the block from its name
   * @param name The name of the block
   * @return Index of the block with name 'name'
   */
  const unsigned int & getIndexFromName(const std::string & name) const;
  const std::vector<Real> & getVolumes() const { return _volume; }
  FunctionName getInitialT() const;

  /**
   * Gets the perimeter of one unit of this heat structure on the specified side
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  virtual Real getUnitPerimeter(const HeatStructureSideType & side) const = 0;

  /**
   * Gets the number of units that heat structure represents
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  Real getNumberOfUnits() const { return _num_rods; }

  /**
   * Return true if this heat structure has the supplied boundary
   */
  bool hasBoundary(const BoundaryName & boundary_name) const;

  /**
   * Get boundary info associated with the heat structure boundary
   *
   * @param[in] boundary  Boundary name of a heat structure boundary
   *
   * @return The list of tuples (element id, local side id) that is associated with boundary
   * `boundary`
   */
  const std::vector<std::tuple<dof_id_type, unsigned short int>> &
  getBoundaryInfo(const BoundaryName & boundary_name) const;

  /**
   * Get boundary info associated with the heat structure side
   *
   * @return The list of tuples (element id, local side id) that is associated with side `side`
   */
  const std::vector<std::tuple<dof_id_type, unsigned short int>> &
  getBoundaryInfo(const HeatStructureSideType & side) const;

  /**
   * Sets the flag specifying that the heat structure is connected to a flow channel to 'true'
   */
  void setConnectedToFlowChannel() const { _connected_to_flow_channel = true; }

  /**
   * Gets the axial offset for the mesh
   */
  Real getAxialOffset() const { return _axial_offset; }

protected:
  virtual std::shared_ptr<HeatConductionModel> buildModel();
  virtual void init() override;
  virtual void check() const override;
  virtual bool usingSecondOrderMesh() const override;

  void build2DMesh();
  void build2DMesh2ndOrder();

  void
  loadMaterial(InputParameters & pars, const std::string & par, const std::string & material_name);

  /// The heat conduction model used by this heat structure
  std::shared_ptr<HeatConductionModel> _hc_model;

  /// Number of heat structures, i.e., for typical PWR, 3 (clad, gap, and fuel).
  unsigned int _number_of_hs;
  /// Name of heat structure parts.
  std::vector<std::string> _names;
  /// Map from block name to block index
  std::map<std::string, unsigned int> _name_index;
  /// Material names
  std::vector<std::string> _material_names;
  /// width(radius) of each heat structure
  std::vector<Real> _width;
  /// Total width of all regions
  Real _total_width;
  /// volume of each heat structure
  std::vector<Real> _volume;
  /// Number of elements in each heat structure
  std::vector<unsigned int> _n_part_elems;
  /// Total elements number for each slice of heat structure
  unsigned int _total_elem_number;
  /// The number of rods represented by this heat structure
  Real _num_rods;

  // mesh related
  /// BC ID of the heat structure (outer)
  std::vector<unsigned int> _outer_bc_id;
  /// BC ID of the heat structure (inner)
  std::vector<unsigned int> _inner_bc_id;
  /// BC ID of the heat structure (start)
  std::vector<unsigned int> _start_bc_id;
  /// BC ID of the heat structure (end)
  std::vector<unsigned int> _end_bc_id;
  /// BC ID of the interior axial boundaries (per radial section) of the heat structure
  std::vector<unsigned int> _interior_axial_per_radial_section_bc_id;
  /// BC ID of the axial regions of the outer boundary of the heat structure
  std::vector<unsigned int> _axial_outer_bc_id;
  /// BC ID of the axial regions of the inner boundary of the heat structure
  std::vector<unsigned int> _axial_inner_bc_id;
  /// BC ID of the radial regions of the start boundary of the heat structure
  std::vector<unsigned int> _radial_start_bc_id;
  /// BC ID of the radial regions of the end boundary of the heat structure
  std::vector<unsigned int> _radial_end_bc_id;
  /// BC ID of the inner radial boundary regions of the heat structure
  std::vector<unsigned int> _inner_radial_bc_id;
  /// Boundary names of the outer side of the heat structure
  std::vector<BoundaryName> _boundary_names_outer;
  /// Boundary names of the inner side of the heat structure
  std::vector<BoundaryName> _boundary_names_inner;
  /// Boundary names of the start side of the heat structure
  std::vector<BoundaryName> _boundary_names_start;
  /// Boundary names of the end side of the heat structure
  std::vector<BoundaryName> _boundary_names_end;
  /// Boundary names of the interior axial boundaries (per radial section) of the heat structure
  std::vector<BoundaryName> _boundary_names_interior_axial_per_radial_section;
  /// Boundary names of the axial regions of the outer side of the heat structure
  std::vector<BoundaryName> _boundary_names_axial_outer;
  /// Boundary names of the axial regions of the inner side of the heat structure
  std::vector<BoundaryName> _boundary_names_axial_inner;
  /// Boundary names of the radial regions of the start side of the heat structure
  std::vector<BoundaryName> _boundary_names_radial_start;
  /// Boundary names of the radial regions of the end side of the heat structure
  std::vector<BoundaryName> _boundary_names_radial_end;
  /// Boundary names of the inner radial boundary regions of the heat structure
  std::vector<BoundaryName> _boundary_names_inner_radial;
  /// Nodes on the side of the "block"
  std::map<std::string, std::vector<unsigned int>> _side_heat_node_ids;
  /// Nodes at the outer side of the generated heat structure
  std::vector<unsigned int> _outer_heat_node_ids;
  /// Nodes at the inner side of the generated heat structure
  std::vector<unsigned int> _inner_heat_node_ids;
  /// Map of boundary name to list of tuples of element and side IDs for that boundary
  std::map<BoundaryName, std::vector<std::tuple<dof_id_type, unsigned short int>>>
      _hs_boundary_info;

  /// True if this heat structure is connected to at least one flow channel
  mutable bool _connected_to_flow_channel;
  /// Distance by which to offset the mesh from the heat structure axis
  mutable Real _axial_offset;

public:
  /// map of heat structure side string to enum
  static const std::map<std::string, HeatStructureSideType> _side_type_to_enum;

  /**
   * Gets a MooseEnum for heat structure side
   *
   * @param[in] name   default value
   * @returns MooseEnum for heat structure side type
   */
  static MooseEnum getSideType(const std::string & name = "");

  static InputParameters validParams();
};

namespace THM
{
template <>
HeatStructureSideType stringToEnum(const std::string & s);
}
