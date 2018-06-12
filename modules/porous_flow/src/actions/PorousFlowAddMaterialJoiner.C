//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAddMaterialJoiner.h"
#include "AddMaterialAction.h"
#include "AddUserObjectAction.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"

registerMooseAction("PorousFlowApp", PorousFlowAddMaterialJoiner, "add_joiners");

template <>
InputParameters
validParams<PorousFlowAddMaterialJoiner>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Adds PorousFlowJoiner materials as required for each phase-dependent property");
  return params;
}

PorousFlowAddMaterialJoiner::PorousFlowAddMaterialJoiner(const InputParameters & params)
  : Action(params),
    _density_nodal(true),
    _density_qp(true),
    _viscosity_nodal(true),
    _viscosity_qp(true),
    _enthalpy_nodal(true),
    _enthalpy_qp(true),
    _internal_energy_nodal(true),
    _internal_energy_qp(true),
    _relperm_nodal(true),
    _relperm_qp(true)
{
}

void
PorousFlowAddMaterialJoiner::act()
{
  // This task only runs after the UserObject and material actions have been added,
  // so we can get the name of the PorousFlowDictator UserObject and all material
  // types that are added in the input file
  if (_current_task == "add_joiners")
  {
    // Get the user objects that have been added to get the name of the PorousFlowDictator
    auto userobjects = _awh.getActions<AddUserObjectAction>();
    for (auto & userobject : userobjects)
      if (userobject->getMooseObjectType() == "PorousFlowDictator")
        _dictator_name = userobject->name();

    // Get the list of materials that have been added
    auto actions = _awh.getActions<AddMaterialAction>();

    // Check if there are PorousFlowJoiner materials in the input file
    checkJoiner();

    for (auto & action : actions)
    {
      AddMaterialAction * material = const_cast<AddMaterialAction *>(action);

      // Input parameters for the material that is being added
      InputParameters & pars = material->getObjectParams();

      // Only check PorousFlowMaterial's
      if (pars.isParamValid("pf_material_type"))
      {
        const std::string pf_material_type = pars.get<std::string>("pf_material_type");

        // Check if the material is evaluated at the nodes or qps
        const bool at_nodes = pars.get<bool>("at_nodes");

        // Add joiner material for fluid properties materials
        if (pf_material_type == "fluid_properties")
        {
          // Key the addition of the joiner off the phase 0 fluid so it is only added once
          if (pars.get<unsigned int>("phase") == 0)
          {
            // Join density and viscosity if they are calculated
            if (pars.get<bool>("compute_density_and_viscosity"))
            {
              if (at_nodes)
              {
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_density_nodal",
                          "PorousFlow_density_nodal_all",
                          _density_nodal);
                addJoiner(at_nodes,
                          "PorousFlow_viscosity_nodal",
                          "PorousFlow_viscosity_nodal_all",
                          _viscosity_nodal);
              }
              else
              {
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_density_qp",
                          "PorousFlow_density_qp_all",
                          _density_qp);
                addJoiner(at_nodes,
                          "PorousFlow_viscosity_qp",
                          "PorousFlow_viscosity_qp_all",
                          _viscosity_qp);
              }
            }

            // Join enthalpy if it is calculated
            if (pars.get<bool>("compute_enthalpy"))
            {
              if (at_nodes)
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_enthalpy_nodal",
                          "PorousFlow_enthalpy_nodal_all",
                          _enthalpy_nodal);
              else
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_enthalpy_qp",
                          "PorousFlow_enthalpy_qp_all",
                          _enthalpy_qp);
            }

            // Join internal energy if it is calculated
            if (pars.get<bool>("compute_internal_energy"))
            {
              if (at_nodes)
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_internal_energy_nodal",
                          "PorousFlow_internal_energy_nodal_all",
                          _internal_energy_nodal);
              else
                addJoiner(at_nodes,
                          "PorousFlow_fluid_phase_internal_energy_qp",
                          "PorousFlow_internal_energy_qp_all",
                          _internal_energy_qp);
            }
          }
        }

        // Add joiner materials for relative permeability materials
        if (pf_material_type == "relative_permeability")
        {
          // Key the addition of the joiner off the phase 0 fluid so it is only added once
          if (pars.get<unsigned int>("phase") == 0)
          {
            if (at_nodes)
              addJoiner(at_nodes,
                        "PorousFlow_relative_permeability_nodal",
                        "PorousFlow_relative_permeability_nodal_all",
                        _relperm_nodal);
            else
              addJoiner(at_nodes,
                        "PorousFlow_relative_permeability_qp",
                        "PorousFlow_relative_permeability_qp_all",
                        _relperm_qp);
          }
        }
      }
    }
  }
}

void
PorousFlowAddMaterialJoiner::addJoiner(bool at_nodes,
                                       const std::string & material_property,
                                       const std::string & output_name,
                                       bool active)
{
  if (active)
  {
    std::string material_type = "PorousFlowJoiner";
    InputParameters params = _factory.getValidParams(material_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("at_nodes") = at_nodes;
    params.set<std::string>("material_property") = material_property;
    _problem->addMaterial(material_type, output_name, params);
  }
}

void
PorousFlowAddMaterialJoiner::checkJoiner()
{
  // Get the list of materials that have been added
  auto actions = _awh.getActions<AddMaterialAction>();

  for (auto & action : actions)
  {
    AddMaterialAction * material = const_cast<AddMaterialAction *>(action);

    if (material->getMooseObjectType() == "PorousFlowJoiner")
    {
      // If a PorousFlowJoiner material has been included in the input file,
      // let the user know that it should be removed
      mooseDeprecated("PorousFlowJoiner materials are no longer required in the input "
                      "file.\nPlease remove all PorousFlowJoiner materials from this input file");

      // Input parameters for the PorousFlowJoiner
      InputParameters & pars = material->getObjectParams();
      const std::string prop = pars.get<std::string>("material_property");

      // Set flag corresponding to the material property that has been found here
      // so that a PorousFlowJoiner isn't added for it in this action
      if (prop == "PorousFlow_fluid_phase_density_nodal")
        _density_nodal = false;
      else if (prop == "PorousFlow_fluid_phase_density_qp")
        _density_qp = false;
      if (prop == "PorousFlow_viscosity_nodal")
        _viscosity_nodal = false;
      else if (prop == "PorousFlow_viscosity_qp")
        _viscosity_qp = false;
      if (prop == "PorousFlow_fluid_phase_enthalpy_nodal")
        _enthalpy_nodal = false;
      else if (prop == "PorousFlow_fluid_phase_enthalpy_qp")
        _enthalpy_qp = false;
      if (prop == "PorousFlow_fluid_phase_internal_energy_nodal")
        _internal_energy_nodal = false;
      else if (prop == "PorousFlow_fluid_phase_internal_energy_qp")
        _internal_energy_qp = false;
      else if (prop == "PorousFlow_relative_permeability_nodal")
        _relperm_nodal = false;
      else if (prop == "PorousFlow_relative_permeability_qp")
        _relperm_qp = false;
      else
      {
      }
    }
  }
}
