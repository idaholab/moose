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

class PorousFlowAddMaterialJoiner;

template <>
InputParameters validParams<PorousFlowAddMaterialJoiner>();

/**
 * Action to programatically add PorousFlowJoiner materials without having
 * to manually enter them in the input file
 */
class PorousFlowAddMaterialJoiner : public Action
{

public:
  PorousFlowAddMaterialJoiner(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * Adds a PorousFlowJoiner for the given material property
   * @param at_nodes if true: produce a nodal material, otherwise: produce a qp material
   * @param material_property join this PorousFlow material property
   * @param output_name The unique name given to this PorousFlowJoiner in the input file
   */
  void
  addJoiner(bool at_nodes, const std::string & material_property, const std::string & output_name);

  /**
   * Helper method to determine if a PorousFLowJoiner material is already present
   * in the input file for the given material property
   * @param property the material property to check
   * @return true if a PorousFLowJoiner is already present for property, false otherwise
   */
  bool hasJoiner(std::string property);

  /// Name of the PorousFlowDictator
  std::string _dictator_name;

  /// Vector of already joined materials (to avoid joining them again)
  std::vector<std::string> _already_joined;
};

