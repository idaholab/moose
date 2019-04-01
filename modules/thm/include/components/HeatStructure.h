#ifndef HEATSTRUCTURE_H
#define HEATSTRUCTURE_H

#include "GeometricalComponent.h"
#include "HeatConductionModel.h"

class HeatStructure;

template <>
InputParameters validParams<HeatStructure>();

class HeatStructure : public GeometricalComponent
{
public:
  HeatStructure(const InputParameters & params);

  /// Heat structure geometry type
  enum EHeatStructureType
  {
    PLATE,   ///< Plate geometry
    CYLINDER ///< Cylinder geometry
  };
  /// Map of heat structure geometry type string to enum
  static const std::map<std::string, EHeatStructureType> _hs_type_to_enum;

  /**
   * Gets a MooseEnum for heat structure geometry type
   *
   * @param[in] name   default value
   * @returns MooseEnum for heat structure geometry type
   */
  static MooseEnum getHeatStructureType(const std::string & name = "PLATE");

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  const std::vector<unsigned int> & getSideNodeIds(const std::string & name) const;
  const std::vector<unsigned int> & getTopNodeIds() const;
  const std::vector<BoundaryName> & getTopBoundaryNames() const;
  const std::vector<BoundaryName> & getBottomBoundaryNames() const;

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
  EHeatStructureType getHSType() const { return _hs_type; }
  unsigned int getDimension() const { return _dim; }
  FunctionName getInitialT() const;
  Real getAxialOffset() const { return _axial_offset; }

  /**
   * Returns true if the heat structure is cylindrical
   *
   * @returns True if heat structure is cylindrical
   */
  bool isCylindrical() const { return _hs_type == CYLINDER; }

  /**
   * Gets the perimeter of one unit of this heat structure on the specified side
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  Real getUnitPerimeter(const MooseEnum & side) const;

  /**
   * Gets the number of units that heat structure represents
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  Real getNumberOfUnits() const { return _num_rods; }

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
  /// Dimension of the mesh
  const unsigned int & _dim;
  /// Heat structure type: either "plate" (default) or "cylinder"
  const EHeatStructureType _hs_type;
  /// Axial offset for the undisplaced mesh
  const Real & _axial_offset;

  /// Number of heat structures, i.e., for typical PWR, 3 (clad, gap, and fuel).
  unsigned int _number_of_hs;
  /// Name of heat structure parts.
  const std::vector<std::string> & _names;
  /// Map from block name to block index
  std::map<std::string, unsigned int> _name_index;
  /// Material names
  const std::vector<std::string> & _material_names;
  /// width(radius) of each heat structure
  const std::vector<Real> & _width;
  /// Total width of all regions
  const Real _total_width;
  /// plate fuel depth
  const Real & _depth;
  /// Perimeter of "top" side of one unit of heat structure
  const Real _P_unit_top;
  /// Perimeter of "bottom" side of one unit of heat structure
  const Real _P_unit_bottom;
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
  /// BC ID of the heat structure (top)
  std::vector<unsigned int> _top_bc_id;
  /// BC ID of the heat structure (bottom)
  std::vector<unsigned int> _bottom_bc_id;
  /// Boundary names of the top of the heat structure
  std::vector<BoundaryName> _boundary_names_top;
  /// Boundary names of the bottom of the heat structure
  std::vector<BoundaryName> _boundary_names_bottom;
  /// Nodes generated for hs (heat structure)
  std::vector<std::vector<unsigned int>> _node_ids;
  /// Nodes on the side of the "block"
  std::map<std::string, std::vector<unsigned int>> _side_heat_node_ids;
  /// Nodes at he top of the generated heat structure
  std::vector<unsigned int> _top_heat_node_ids;
  /// Nodes at he top of the generated heat structure
  std::vector<unsigned int> _bottom_heat_node_ids;
  /// Elements generated for hs (heat structure)
  std::vector<std::vector<unsigned int>> _elem_ids;
};

namespace THM
{
template <>
HeatStructure::EHeatStructureType stringToEnum(const std::string & s);
}

#endif /* HEATSTRUCTURE_H */
