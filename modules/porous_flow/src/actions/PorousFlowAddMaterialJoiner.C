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
#include "ActionWarehouse.h"
#include "FEProblem.h"
#include "Material.h"
#include "UserObject.h"
#include "PorousFlowDictator.h"

registerMooseAction("PorousFlowApp", PorousFlowAddMaterialJoiner, "add_joiners");

InputParameters
PorousFlowAddMaterialJoiner::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Adds PorousFlowJoiner materials as required for each phase-dependent property");
  return params;
}

PorousFlowAddMaterialJoiner::PorousFlowAddMaterialJoiner(const InputParameters & params)
  : Action(params), _already_joined()
{
}

void
PorousFlowAddMaterialJoiner::act()
{
  // This task only runs after the UserObject and material actions have run,
  // so we can get the name of the PorousFlowDictator UserObject and all material
  // types
  if (_current_task == "add_joiners")
  {
    // Get the user objects that have been added to get the name of the PorousFlowDictator
    std::vector<UserObject *> userobjects;
    _problem->theWarehouse()
        .query()
        .condition<AttribSystem>("UserObject")
        .condition<AttribThread>(0)
        .queryInto(userobjects);
    for (auto & userobject : userobjects)
      if (dynamic_cast<PorousFlowDictator *>(userobject))
        _dictator_name = userobject->name();

    // Get the list of materials that have been added
    auto materials = _problem->getMaterialWarehouse().getObjects();
    for (auto & mat : materials)
    {
      const InputParameters & params = mat->parameters();

      // Only check PorousFlowMaterials
      if (params.isParamValid("pf_material_type"))
      {
        const std::string pf_material_type = params.get<std::string>("pf_material_type");

        _block_restricted = params.isParamValid("block");
        if (_block_restricted &&
            (pf_material_type == "fluid_properties" || pf_material_type == "relative_permeability"))
        {
          // this Material is block-restricted, and we're about to add Joiners for it, so must get
          // a union of all the blocks for this material type in order to block-restrict the Joiner
          std::set<SubdomainName> unique_blocks;
          for (auto & mat2 : _problem->getMaterialWarehouse().getObjects())
          {
            const InputParameters & params2 = mat2->parameters();
            if (params2.isParamValid("pf_material_type") &&
                params2.get<std::string>("pf_material_type") == pf_material_type)
            {
              if (!params2.isParamValid("block"))
              {
                // a Material of this type is not block-restricted, so the non-block-restricted
                // Joiner can be added
                _block_restricted = false;
                break;
              }
              const std::vector<SubdomainName> & bl =
                  params2.get<std::vector<SubdomainName>>("block");
              std::copy(bl.begin(), bl.end(), std::inserter(unique_blocks, unique_blocks.end()));
            }
          }
          _blocks.assign(unique_blocks.begin(), unique_blocks.end());
        }

        // Check if the material is evaluated at the nodes or qps
        const bool at_nodes = params.get<bool>("at_nodes");

        // Add joiner material for fluid properties materials
        if (pf_material_type == "fluid_properties")
        {
          // Check if the material defines AD material properties
          const bool is_ad = params.get<bool>("is_ad");

          // Key the addition of the joiner off the phase 0 fluid so it is only added once
          if (params.get<unsigned int>("phase") == 0)
          {
            // Join density and viscosity if they are calculated
            if (params.get<bool>("compute_density_and_viscosity"))
            {
              if (at_nodes)
              {
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_density_nodal",
                          "PorousFlow_density_nodal_all");
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_viscosity_nodal",
                          "PorousFlow_viscosity_nodal_all");
              }
              else
              {
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_density_qp",
                          "PorousFlow_density_qp_all");
                addJoiner(
                    at_nodes, is_ad, "PorousFlow_viscosity_qp", "PorousFlow_viscosity_qp_all");
              }
            }

            // Join enthalpy if it is calculated
            if (params.get<bool>("compute_enthalpy"))
            {
              if (at_nodes)
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_enthalpy_nodal",
                          "PorousFlow_enthalpy_nodal_all");
              else
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_enthalpy_qp",
                          "PorousFlow_enthalpy_qp_all");
            }

            // Join internal energy if it is calculated
            if (params.get<bool>("compute_internal_energy"))
            {
              if (at_nodes)
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_internal_energy_nodal",
                          "PorousFlow_internal_energy_nodal_all");
              else
                addJoiner(at_nodes,
                          is_ad,
                          "PorousFlow_fluid_phase_internal_energy_qp",
                          "PorousFlow_internal_energy_qp_all");
            }
          }
        }

        // Add joiner materials for relative permeability materials
        if (pf_material_type == "relative_permeability")
        {
          // Check if the material defines AD material properties
          const bool is_ad = params.get<bool>("is_ad");

          // Key the addition of the joiner off the phase 0 fluid so it is only added once
          if (params.get<unsigned int>("phase") == 0)
          {
            if (at_nodes)
              addJoiner(at_nodes,
                        is_ad,
                        "PorousFlow_relative_permeability_nodal",
                        "PorousFlow_relative_permeability_nodal_all");
            else
              addJoiner(at_nodes,
                        is_ad,
                        "PorousFlow_relative_permeability_qp",
                        "PorousFlow_relative_permeability_qp_all");
          }
        }
      }
    }
  }
}

void
PorousFlowAddMaterialJoiner::addJoiner(bool at_nodes,
                                       bool is_ad,
                                       const std::string & material_property,
                                       const std::string & output_name)
{
  bool is_joined = false;

  // Check if this material is already joined
  if (std::find(_already_joined.begin(), _already_joined.end(), material_property) !=
      _already_joined.end())
    is_joined = true;

  if (hasJoiner(material_property))
    is_joined = true;

  if (!is_joined)
  {
    std::string material_type;
    is_ad ? material_type = "ADPorousFlowJoiner" : material_type = "PorousFlowJoiner";
    InputParameters params = _factory.getValidParams(material_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("at_nodes") = at_nodes;
    params.set<std::string>("material_property") = material_property;
    if (_block_restricted)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    _problem->addMaterial(material_type, output_name, params);

    // Add material to the already joined list
    _already_joined.push_back(material_property);
  }
}

bool
PorousFlowAddMaterialJoiner::hasJoiner(std::string property)
{
  // Get the list of materials in the input file
  auto actions = _awh.getActions<AddMaterialAction>();

  for (auto & action : actions)
  {
    AddMaterialAction * material = const_cast<AddMaterialAction *>(action);

    if (material->getMooseObjectType() == "PorousFlowJoiner")
    {
      // If a PorousFlowJoiner material has been included in the input file,
      // let the user know that it should be removed
      mooseDeprecated("PorousFlowJoiner materials are no longer required in the input "
                      "file.\nPlease remove all PorousFlowJoiner materials from this input file to "
                      "get rid of this warning");

      const std::string joiner_property =
          material->getObjectParams().get<std::string>("material_property");

      // Check if the given material property is joined by this material
      if (joiner_property == property)
        return true;
    }
  }

  // If no PorousFlowJoiner materials matched property, return false
  return false;
}
