//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAddMaterialAction.h"
#include "AddBCAction.h"
#include "AddDiracKernelAction.h"
#include "AddKernelAction.h"
#include "AddMaterialAction.h"
#include "AddPostprocessorAction.h"
#include "AddUserObjectAction.h"
#include "PorousFlowActionBase.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "FEProblem.h"
#include "MooseObjectAction.h"
#include "Conversion.h"

registerMooseAction("PorousFlowApp", PorousFlowAddMaterialAction, "meta_action");

InputParameters
PorousFlowAddMaterialAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Makes sure that the correct nodal and/or qp materials are added for each property");
  return params;
}

PorousFlowAddMaterialAction::PorousFlowAddMaterialAction(const InputParameters & params)
  : Action(params), PorousFlowDependencies()
{
}

void
PorousFlowAddMaterialAction::act()
{
  // Create the list of kernels, auxkernels, actions etc that each material may be
  // required by
  createDependencyList();

  // Get the list of materials that have been added
  auto actions = _awh.getActions<AddMaterialAction>();

  for (auto & action : actions)
    _ama_materials.push_back(const_cast<AddMaterialAction *>(action));

  for (auto & material : _ama_materials)
  {
    InputParameters & pars = material->getObjectParams();

    // Check if the material is a PorousFlow material
    if (pars.isParamValid("pf_material_type"))
    {
      const std::string pf_material_type = pars.get<std::string>("pf_material_type");

      // PorousFlowJoiner materials are added automatically by the PorousFlowAddMaterialJoiner
      // action, so no need to check these here
      if (pf_material_type != "joiner")
      {
        // There are two possibilities that must be considered:
        // 1) The parameter at_nodes has been set by the user. In this case, the material will
        // be added as normal by AddMaterialAction
        // 2) The parameter at_nodes has not been set by the user. In this case, this action
        // will check to see if the material is required at the qps, at the nodes, or possibly both

        // Only check the second possibility
        if (!pars.isParamSetByUser("at_nodes"))
        {
          bool qp_material_required = false;

          // First, check the case at_nodes = false, as this is the default behaviour for the
          // at_nodes parameter. Note: the local variable at_nodes is set to true, so the material
          // is at the qps when !at_nodes
          const bool at_nodes = true;

          if (isPFMaterialRequired(pf_material_type, !at_nodes))
          {
            // This material is required at the qps, so add it as normal (setting the paramter
            // at_nodes = false for clarity)
            pars.set<bool>("at_nodes") = !at_nodes;
            qp_material_required = true;
          }

          // Check if the material is required at the nodes as well and that it isn't already
          // added in the input file. If it is needed and not already supplied, then it is added
          // in one of two ways: 1) If the material wasn't also required at the qps (checked above),
          // then we can simply set the at_nodes parameter to true. 2) If it was also required at
          // the qps, then a new material action is required to be added to the action warehouse
          if (isPFMaterialRequired(pf_material_type, at_nodes) &&
              !isPFMaterialPresent(material, at_nodes))
          {
            if (!qp_material_required)
              pars.set<bool>("at_nodes") = at_nodes;
            else
              addPFMaterial(material, at_nodes);
          }
        }
      }
    }
  }
}

void
PorousFlowAddMaterialAction::createDependencyList()
{
  // Unique list of kernels added in input file
  auto kernels = _awh.getActions<AddKernelAction>();
  for (auto & kernel : kernels)
    _dependency_list.insert(kernel->getMooseObjectType());

  // Unique list of PorousFlowActions added in input file
  auto actions = _awh.getActions<PorousFlowActionBase>();
  for (auto & action : actions)
    _dependency_list.insert(action->name());

  // Unique list of auxkernels added in input file
  auto auxkernels = _awh.getActions<AddKernelAction>();
  for (auto & auxkernel : auxkernels)
    _dependency_list.insert(auxkernel->getMooseObjectType());

  // Unique list of postprocessors added in input file
  auto postprocessors = _awh.getActions<AddPostprocessorAction>();
  for (auto & postprocessor : postprocessors)
    _dependency_list.insert(postprocessor->getMooseObjectType());

  // Unique list of userojects added in input file
  auto userobjects = _awh.getActions<AddUserObjectAction>();
  for (auto & userobject : userobjects)
    _dependency_list.insert(userobject->getMooseObjectType());

  // Unique list of BCs added in input file
  auto bcs = _awh.getActions<AddBCAction>();
  for (auto & bc : bcs)
    _dependency_list.insert(bc->getMooseObjectType());

  // Unique list of Dirac kernels added in input file
  auto diracs = _awh.getActions<AddDiracKernelAction>();
  for (auto & dirac : diracs)
    _dependency_list.insert(dirac->getMooseObjectType());
}

bool
PorousFlowAddMaterialAction::isPFMaterialRequired(std::string pf_material_type, bool at_nodes)
{
  const std::string nodal_ext = at_nodes ? "_nodal" : "_qp";

  // Check if this material is required by looping through the list of dependencies
  bool required = false;
  for (auto item : _dependency_list)
  {
    required = _deps.dependsOn(item, pf_material_type + nodal_ext);
    if (required)
      break;
  }

  return required;
}

bool
PorousFlowAddMaterialAction::isPFMaterialPresent(AddMaterialAction * material, bool at_nodes)
{
  bool is_present = false;

  // Need to check that it hasn't been added in the input file also to
  // avoid a duplicate material property error
  for (auto & ama_material : _ama_materials)
  {
    if (ama_material->name() != material->name() &&
        ama_material->getMooseObjectType() == material->getMooseObjectType())
    {
      InputParameters & mat_params = ama_material->getObjectParams();
      const bool mat_at_nodes = mat_params.get<bool>("at_nodes");

      InputParameters & pars = material->getObjectParams();

      // If the material isn't related to a fluid phase, it is present if
      // its at_nodes parameter is equal to the given at_nodes
      if (mat_at_nodes == at_nodes && !pars.isParamValid("phase"))
        is_present = true;

      // If the material is related to a fluid phase, it is present if
      // its at_nodes parameter is equal to the given at_nodes, and its
      // phase is equal to phase
      if (pars.isParamValid("phase"))
      {
        const unsigned int phase = pars.get<unsigned int>("phase");

        if (mat_params.isParamValid("phase") && mat_params.get<unsigned int>("phase") == phase)
          is_present = true;
      }

      // Finally, if the material is block restricted then it is not already
      // present if the block parameter is not identical
      if (mat_params.get<std::vector<SubdomainName>>("block") !=
          pars.get<std::vector<SubdomainName>>("block"))
        is_present = false;
    }
  }

  return is_present;
}

void
PorousFlowAddMaterialAction::addPFMaterial(AddMaterialAction * material, bool at_nodes)
{
  const std::string nodal_ext = at_nodes ? "_nodal" : "_qp";

  // Input parameters for the material that is being added
  InputParameters & pars = material->getObjectParams();

  // PorousFlowMaterial type
  const std::string pf_material_type = pars.get<std::string>("pf_material_type");
  const std::string moose_object_type = material->getMooseObjectType();

  // If it is a material that also has a fluid phase, then extract that to add to name
  std::string phase_str;
  if (pars.isParamValid("phase"))
  {
    unsigned int phase = pars.get<unsigned int>("phase");
    phase_str = "_phase" + Moose::stringify(phase);
  }

  // Add material to the action warehouse
  InputParameters action_params = _action_factory.getValidParams("AddMaterialAction");
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Setup action for passed in material type
  action_params.set<std::string>("type") = moose_object_type;

  const std::string material_name = material->name() + phase_str + nodal_ext;

  auto action = MooseSharedNamespace::dynamic_pointer_cast<MooseObjectAction>(
      _action_factory.create("AddMaterialAction", material_name, action_params));

  action->getObjectParams().applyParameters(pars);
  action->getObjectParams().set<bool>("at_nodes") = at_nodes;

  _awh.addActionBlock(action);
}
