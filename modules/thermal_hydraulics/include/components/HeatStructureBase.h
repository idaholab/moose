//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component2D.h"
#include "HeatStructureInterface.h"
#include "HeatConductionModel.h"

/// heat structure side
enum class HeatStructureSideType
{
  INNER = 0,
  OUTER = 1,
  START = 2,
  END = 3
};

/**
 * Base class for 2D generated heat structures
 */
class HeatStructureBase : public Component2D, public HeatStructureInterface
{
public:
  HeatStructureBase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  const std::vector<unsigned int> & getSideNodeIds(const std::string & name) const;
  const std::vector<unsigned int> & getOuterNodeIds() const;
  const std::vector<BoundaryName> & getOuterBoundaryNames() const;
  const std::vector<BoundaryName> & getInnerBoundaryNames() const;
  const std::vector<BoundaryName> & getStartBoundaryNames() const;
  const std::vector<BoundaryName> & getEndBoundaryNames() const;

  /**
   * Get index of the block from its name
   * @param name The name of the block
   * @return Index of the block with name 'name'
   */
  const unsigned int & getIndexFromName(const std::string & name) const;

  /**
   * Returns true if a heat structure side type exists for the given boundary name
   *
   * @param[in] boundary_name   Boundary name for which to get heat structure side type
   */
  bool hasHeatStructureSideType(const BoundaryName & boundary_name) const;

  /**
   * Gets the heat structure side type of the provided boundary
   *
   * An error is thrown if the supplied boundary name does not exist in this
   * component or if the boundary is interior.
   *
   * @param[in] boundary_name   Boundary name for which to get heat structure side type
   */
  HeatStructureSideType getHeatStructureSideType(const BoundaryName & boundary_name) const;

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

  using Component2D::getBoundaryInfo;
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

protected:
  virtual void init() override;
  virtual void check() const override;
  virtual bool usingSecondOrderMesh() const override;

  void
  loadMaterial(InputParameters & pars, const std::string & par, const std::string & material_name);

  /// Map from block name to block index
  std::map<std::string, unsigned int> _name_index;
  /// Material names
  std::vector<std::string> _material_names;
  /// The number of rods represented by this heat structure
  Real _num_rods;

  /// True if this heat structure is connected to at least one flow channel
  mutable bool _connected_to_flow_channel;

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
