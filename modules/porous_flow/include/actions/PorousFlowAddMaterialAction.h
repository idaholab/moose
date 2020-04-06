//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "PorousFlowDependencies.h"

class AddMaterialAction;

/**
 * Action to automatically ensure that PorousFlowMaterials are correctly evaluated
 * at either the qps, nodes, or both
 *
 * This action works by checking all materials added in the input file. If a material
 * is a valid PorousFlowMaterial, then it does one of the following:
 *   1) If the at_nodes parameter has been set by the user, then it leaves that
 *      material alone (this assumes that the user has correctly added that material).
 *   2) If the at_nodes parameter has not been set by the user, then the action checks to
 *      see if this material is required at the qps, nodes or both, and makes sure that the
 *      correct versions are added.
 *   3) If a PorousFlowJoiner material is included in the input file, it does nothing (as
 *      the PorousFlowAddMaterialJoiner action will check for these and give a message that
 *      these materials are no longer required in the input file)
 */
class PorousFlowAddMaterialAction : public Action, public PorousFlowDependencies
{

public:
  static InputParameters validParams();

  PorousFlowAddMaterialAction(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * Creates a set of all actions, kernels, etc to check material dependency
   * against in order to determine whether to add nodal and/or qp materials
   */
  void createDependencyList();

  /**
   * Check to see if the material with a given at_nodes parameter is required
   * @param at_nodes true if material is evaluated at the nodes, false otherwise
   * @return true is the material is required, false otherwise
   */
  bool isPFMaterialRequired(std::string pf_material_type, bool at_nodes);

  /**
   * Check to see if the material with a given at_nodes parameter has already
   * been included in the input file (to void duplicate material property errors)
   * @param material pointer to the material in the action warehouse
   * @param at_nodes true if material is evaluated at the nodes, false otherwise
   * @return true is the material is already added in the input file, false otherwise
   */
  bool isPFMaterialPresent(AddMaterialAction * material, bool at_nodes);

  /**
   * Adds the material for the given at_nodes parameter
   * @param material pointer to the material in the action warehouse
   * @param at_nodes true if material is evaluates at the nodes, false otherwise
   */
  void addPFMaterial(AddMaterialAction * material, bool at_nodes);

  /// Name of the PorousFlowDictator
  std::string _dictator_name;
  /// List of kernels, actions etc that may depend on PorousFlow materials
  std::set<std::string> _dependency_list;
  /// List of all materials added in the input file by AddMaterialAction
  std::vector<AddMaterialAction *> _ama_materials;
};
