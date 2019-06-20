#pragma once

#include "GeometricalComponent.h"
#include "HeatConductionModel.h"

class HeatStructureBase;

template <>
InputParameters validParams<HeatStructureBase>();

class HeatStructureBase : public GeometricalComponent
{
public:
  HeatStructureBase(const InputParameters & params);

  /// heat structure side
  enum SideType
  {
    INNER = 0,
    OUTER = 1
  };

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  const std::vector<unsigned int> & getSideNodeIds(const std::string & name) const;
  const std::vector<unsigned int> & getOuterNodeIds() const;
  const std::vector<BoundaryName> & getOuterBoundaryNames() const;
  const std::vector<BoundaryName> & getInnerBoundaryNames() const;

  const Real & getTotalWidth() const { return _total_width; }
  unsigned int getNumHS() const { return _number_of_hs; }

  /**
   * Check that the block with name 'name' is defined in the heat structure
   * @param name The name to be looked for
   * @return true if the heat structure has a block with name 'name', otherwise false
   */
  bool hasBlock(const std::string & name) const;

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
  virtual Real getUnitPerimeter(const HeatStructureBase::SideType & side) const = 0;

  /**
   * Gets the number of units that heat structure represents
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  Real getNumberOfUnits() const { return _num_rods; }

  /**
   * Get the axial offset when generating a heat structure mesh
   *
   * @returns The axial offset of the mesh
   */
  virtual Real getAxialOffset() const;

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
  const std::vector<std::string> & _names;
  /// Names of each axial region
  const std::vector<std::string> & _axial_region_names;
  /// Map from block name to block index
  std::map<std::string, unsigned int> _name_index;
  /// Material names
  const std::vector<std::string> & _material_names;
  /// width(radius) of each heat structure
  const std::vector<Real> & _width;
  /// Total width of all regions
  const Real _total_width;
  /// volume of each heat structure
  std::vector<Real> _volume;
  /// Number of elements in each heat structure
  const std::vector<unsigned int> & _n_part_elems;
  /// Total elements number for each slice of heat structure
  unsigned int _total_elem_number;
  /// The number of rods represented by this heat structure
  const Real & _num_rods;

  /// Thermal conductivity of each heat structure part
  const bool _has_k;
  /// Heat capacity of each heat structure part
  const bool _has_Cp;
  /// Density of each heat structure part
  const bool _has_rho;

  // mesh related
  /// Subdomain id this flow channel defined
  unsigned int _subdomain_id;
  /// BC ID of the heat structure (outer)
  std::vector<unsigned int> _outer_bc_id;
  /// BC ID of the heat structure (inner)
  std::vector<unsigned int> _inner_bc_id;
  /// Boundary names of the outer side of the heat structure
  std::vector<BoundaryName> _boundary_names_outer;
  /// Boundary names of the inner side of the heat structure
  std::vector<BoundaryName> _boundary_names_inner;
  /// Nodes on the side of the "block"
  std::map<std::string, std::vector<unsigned int>> _side_heat_node_ids;
  /// Nodes at the outer side of the generated heat structure
  std::vector<unsigned int> _outer_heat_node_ids;
  /// Nodes at the inner side of the generated heat structure
  std::vector<unsigned int> _inner_heat_node_ids;

public:
  /// map of heat structure side string to enum
  static const std::map<std::string, SideType> _side_type_to_enum;

  /**
   * Gets a MooseEnum for heat structure side
   *
   * @param[in] name   default value
   * @returns MooseEnum for heat structure side type
   */
  static MooseEnum getSideType(const std::string & name = "");
};

namespace THM
{
template <>
HeatStructureBase::SideType stringToEnum(const std::string & s);
}
