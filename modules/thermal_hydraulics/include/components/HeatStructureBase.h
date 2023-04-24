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

/**
 * Base class for 2D generated heat structures
 */
class HeatStructureBase : public Component2D, public HeatStructureInterface
{
public:
  HeatStructureBase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /**
   * Get index of the block from its name
   * @param name The name of the block
   * @return Index of the block with name 'name'
   */
  const unsigned int & getIndexFromName(const std::string & name) const;

  /**
   * Gets the perimeter of one unit of this heat structure on the specified side
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  virtual Real getUnitPerimeter(const ExternalBoundaryType & side) const = 0;

  /**
   * Gets the number of units that heat structure represents
   *
   * @param[in] side   Side of the heat structure corresponding to desired perimeter
   * @returns Perimeter of one unit of this heat structure on the specified side
   */
  Real getNumberOfUnits() const { return _num_rods; }

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

  // This reference should be deleted after applications start using _n_regions:
  unsigned int & _number_of_hs;

public:
  static InputParameters validParams();
};
