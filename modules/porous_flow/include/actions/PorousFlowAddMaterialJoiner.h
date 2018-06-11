//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWADDMATERIALJOINER_H
#define POROUSFLOWADDMATERIALJOINER_H

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
   * @param active if true: add PorousFlowJoiner, otherwise: do nothing
   */
  void addJoiner(bool at_nodes,
                 const std::string & material_property,
                 const std::string & output_name,
                 bool active = true);

  /**
   * Helper method to determine if any PorousFlowJoiner materials are present in
   * the input file, and if they are, set's flags to disable adding an identical
   * PorousFlowJoiner material in this action (which would otherwise result in a
   * duplciate material property error).
   */
  void checkJoiner();

  /// Name of the PorousFlowDictator
  std::string _dictator_name;
  /// Flags to control adding PorousFlowJoiners for each material
  bool _density_nodal;
  bool _density_qp;
  bool _viscosity_nodal;
  bool _viscosity_qp;
  bool _enthalpy_nodal;
  bool _enthalpy_qp;
  bool _internal_energy_nodal;
  bool _internal_energy_qp;
  bool _relperm_nodal;
  bool _relperm_qp;
};

#endif // POROUSFLOWADDMATERIALJOINER_H
