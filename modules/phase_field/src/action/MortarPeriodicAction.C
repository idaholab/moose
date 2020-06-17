//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarPeriodicAction.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", MortarPeriodicAction, "add_constraint");

registerMooseAction("PhaseFieldApp", MortarPeriodicAction, "add_mesh_modifier");

registerMooseAction("PhaseFieldApp", MortarPeriodicAction, "add_variable");

InputParameters
MortarPeriodicAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add mortar interfaces, Lagrange multiplier variables, and "
                             "constraints to implement mortar based periodicity of values or "
                             "gradients on a MortarPeriodicMesh");
  params.addParam<std::vector<VariableName>>("variable", "Periodic variables");
  MooseEnum periodicity_type("gradient value", "gradient");
  params.addParam<MooseEnum>("periodicity", periodicity_type, "Periodicity type");
  MultiMooseEnum periodic_dirs("x=0 y=1 z=2");
  params.addRequiredParam<MultiMooseEnum>(
      "periodic_directions",
      periodic_dirs,
      "Directions along which additional Mortar meshes are generated");
  return params;
}

MortarPeriodicAction::MortarPeriodicAction(const InputParameters & parameters)
  : Action(parameters),
    _variables(getParam<std::vector<VariableName>>("variable")),
    _periodicity(getParam<MooseEnum>("periodicity")),
    _periodic_directions(getParam<MultiMooseEnum>("periodic_directions"))
{
}

void
MortarPeriodicAction::act()
{
  // mesh dimension
  const unsigned short dim = _mesh->dimension();

  // periodicity subblock name
  std::string periodicity_name = name();

  // axis names
  const std::vector<BoundaryName> axis = {"x", "y", "z"};

  // boundaries
  const std::vector<BoundaryName> boundary_names = {"left", "bottom", "back"};
  // opposite boundaries
  const std::vector<BoundaryName> opposite_boundary_names = {"right", "top", "front"};

  // Get the current subdomain ids
  std::set<SubdomainID> current_subdomain_ids;
  _mesh->getMesh().subdomain_ids(current_subdomain_ids);

  SubdomainID new_subdomain_id =
      *std::max_element(current_subdomain_ids.begin(), current_subdomain_ids.end()) + 1;

  // iterate over the periodic directions
  for (unsigned short i = 0; i < dim; ++i)
  {
    if (_periodic_directions.contains(i))
    {
      //
      // Add Mortar interfaces. I am only going to add these to the reference
      // mesh because this action currently has no machinery to add constraints
      // on the displaced problem (e.g. see the add_constraint block)
      //
      if (_current_task == "add_mesh_modifier")
      {
        // Don't do mesh modifiers when recovering!
        if (!_app.isRecovering())
        {
          auto secondary_params = _factory.getValidParams("LowerDBlockFromSideset");
          auto master_params = _factory.getValidParams("LowerDBlockFromSideset");

          auto secondary_boundary_id = _mesh->getBoundaryID(boundary_names[i]);
          auto master_boundary_id = _mesh->getBoundaryID(opposite_boundary_names[i]);

          secondary_params.set<SubdomainID>("new_block_id") = new_subdomain_id++;
          secondary_params.set<SubdomainName>("new_block_name") = "secondary_" + axis[i];
          secondary_params.set<std::vector<BoundaryID>>("sidesets") = {secondary_boundary_id};
          secondary_params.set<MooseMesh *>("_mesh") = _mesh.get();

          master_params.set<SubdomainID>("new_block_id") = new_subdomain_id++;
          master_params.set<SubdomainName>("new_block_name") = "master_" + axis[i];
          master_params.set<std::vector<BoundaryID>>("sidesets") = {master_boundary_id};
          master_params.set<MooseMesh *>("_mesh") = _mesh.get();

          _app.addMeshModifier("LowerDBlockFromSideset", axis[i] + "_secondary_lower_d", secondary_params);
          _app.addMeshModifier(
              "LowerDBlockFromSideset", axis[i] + "_master_lower_d", master_params);
        }
      }

      // Lagrange multiplier variable base name
      const std::string lm_base = "lm_" + periodicity_name + "_" + boundary_names[i];

      //
      // Add Lagrange multiplier variables
      //

      if (_current_task == "add_variable")
      {
        std::set<SubdomainID> sub_ids = {_mesh->getSubdomainID("secondary_" + axis[i])};
        for (auto & var : _variables)
        {
          switch (_periodicity)
          {
            case 0: // gradient
            {
              for (unsigned short j = 0; j < dim; ++j)
              {
                _problem->addVariable(lm_base + "_" + var + "_d" + axis[j],
                                      FEType(Utility::string_to_enum<Order>("FIRST"),
                                             Utility::string_to_enum<FEFamily>("LAGRANGE")),
                                      1.0,
                                      &sub_ids);
              }
              break;
            }

            case 1: // value
            {
              _problem->addVariable(lm_base + "_" + var,
                                    FEType(Utility::string_to_enum<Order>("FIRST"),
                                           Utility::string_to_enum<FEFamily>("LAGRANGE")),
                                    1.0,
                                    &sub_ids);
              break;
            }

            default:
              paramError("periodicity", "Periodicity type not implemented");
          }
        }
      }

      //
      // Add Constraints
      //

      if (_current_task == "add_constraint")
      {
        for (auto & var : _variables)
        {
          const std::string ct_base =
              "ct_" + periodicity_name + "_" + boundary_names[i] + "_" + var;
          switch (_periodicity)
          {
            case 0: // gradient
            {
              for (unsigned short j = 0; j < dim; ++j)
              {
                InputParameters params = _factory.getValidParams("EqualGradientConstraint");
                params.set<NonlinearVariableName>("variable") =
                    lm_base + "_" + var + "_d" + axis[j];
                params.set<VariableName>("master_variable") = var;
                params.set<BoundaryName>("secondary_boundary_name") = boundary_names[i];
                params.set<BoundaryName>("master_boundary_name") = opposite_boundary_names[i];
                params.set<SubdomainName>("secondary_subdomain_name") = "secondary_" + axis[i];
                params.set<SubdomainName>("master_subdomain_name") = "master_" + axis[i];
                params.set<unsigned int>("component") = j;
                params.set<bool>("periodic") = true;
                _problem->addConstraint(
                    "EqualGradientConstraint", ct_base + "_d" + axis[j], params);
              }
              break;
            }

            case 1: // value
            {
              InputParameters params = _factory.getValidParams("EqualValueConstraint");
              params.set<NonlinearVariableName>("variable") = lm_base + "_" + var;
              params.set<VariableName>("master_variable") = var;
              params.set<BoundaryName>("secondary_boundary_name") = boundary_names[i];
              params.set<BoundaryName>("master_boundary_name") = opposite_boundary_names[i];
              params.set<SubdomainName>("secondary_subdomain_name") = "secondary_" + axis[i];
              params.set<SubdomainName>("master_subdomain_name") = "master_" + axis[i];
              params.set<bool>("periodic") = true;
              _problem->addConstraint("EqualValueConstraint", ct_base, params);

              break;
            }

            default:
              mooseError("Periodicity type not implemented");
          }
        }
      }
    }
  }
}
