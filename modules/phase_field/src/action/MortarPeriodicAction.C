/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MortarPeriodicAction.h"
#include "MortarPeriodicMesh.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<MortarPeriodicAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Add mortar interfaces, Lagrange multiplier variables, and "
                             "constraints to implement mortar based periodicity of values or "
                             "gradients on a MortarPeriodicMesh");
  params.addParam<std::vector<VariableName>>("variable", "Periodic variables");
  MooseEnum periodicity_type("gradient value", "gradient");
  params.addParam<MooseEnum>("periodicity", periodicity_type, "Periodicity type");
  return params;
}

MortarPeriodicAction::MortarPeriodicAction(const InputParameters & parameters)
  : Action(parameters),
    _variables(getParam<std::vector<VariableName>>("variable")),
    _periodicity(getParam<MooseEnum>("periodicity"))
{
}

void
MortarPeriodicAction::act()
{
  // get the mesh
  MooseSharedPointer<MortarPeriodicMesh> mesh =
      MooseSharedNamespace::dynamic_pointer_cast<MortarPeriodicMesh>(_mesh);
  if (!mesh)
    mooseError("Please use a MortarPeriodicMesh in your simulation.");

  // mesh dimension
  const unsigned short dim = mesh->dimension();

  // periodicity subblock name
  std::string periodicity_name = name();

  // axis names
  const std::vector<BoundaryName> axis = {"x", "y", "z"};

  // boundaries
  const std::vector<BoundaryName> boundary_names = {"left", "bottom", "back"};
  // opposite boundaries
  const std::vector<BoundaryName> opposite_boundary_names = {"right", "top", "front"};

  // mortar subdomains
  const std::vector<SubdomainID> & mortar_subdomains = mesh->getMortarSubdomains();

  // iterate over the periodic directions
  for (unsigned short i = 0; i < dim; ++i)
    if (mesh->getPeriodicDirections().contains(i))
    {
      // initialize subdomain restriction set
      std::set<SubdomainID> subdomain_restriction = {mortar_subdomains[i]};

      // Lagrange multiplier variable base name
      const std::string lm_base = "lm_" + periodicity_name + "_" + boundary_names[i];

      // mortar interface name
      const std::string mi_name =
          "mi_" + periodicity_name + "_" + boundary_names[i] + '_' + opposite_boundary_names[i];

      //
      // Add Lagrange multiplier variables
      //

      if (_current_task == "add_variable")
      {
        for (auto & var : _variables)
        {
          switch (_periodicity)
          {
            case 0: // gradient
              for (unsigned short j = 0; j < dim; ++j)
                _problem->addVariable(lm_base + "_" + var + "_d" + axis[j],
                                      FEType(Utility::string_to_enum<Order>("FIRST"),
                                             Utility::string_to_enum<FEFamily>("LAGRANGE")),
                                      1.0,
                                      &subdomain_restriction);
              break;

            case 1: // value
              _problem->addVariable(lm_base + "_" + var,
                                    FEType(Utility::string_to_enum<Order>("FIRST"),
                                           Utility::string_to_enum<FEFamily>("LAGRANGE")),
                                    1.0,
                                    &subdomain_restriction);
              break;

            default:
              mooseError("Periodicity type not implemented");
          }
        }
      }

      //
      // Add Mortar interfaces
      //

      if (_current_task == "add_mortar_interface")
      {
        _mesh->addMortarInterface(mi_name,
                                  boundary_names[i],
                                  opposite_boundary_names[i],
                                  Moose::stringify(mortar_subdomains[i]));
        if (_displaced_mesh)
          _displaced_mesh->addMortarInterface(mi_name,
                                              boundary_names[i],
                                              opposite_boundary_names[i],
                                              Moose::stringify(mortar_subdomains[i]));
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
              for (unsigned short j = 0; j < dim; ++j)
              {
                InputParameters params = _factory.getValidParams("EqualGradientConstraint");
                params.set<NonlinearVariableName>("variable") =
                    lm_base + "_" + var + "_d" + axis[j];
                params.set<VariableName>("master_variable") = var;
                params.set<std::string>("interface") = mi_name;
                params.set<unsigned int>("component") = j;
                _problem->addConstraint(
                    "EqualGradientConstraint", ct_base + "_d" + axis[j], params);
              }
              break;

            case 1: // value
            {
              InputParameters params = _factory.getValidParams("EqualValueConstraint");
              params.set<NonlinearVariableName>("variable") = lm_base + "_" + var;
              params.set<VariableName>("master_variable") = var;
              params.set<std::string>("interface") = mi_name;
              _problem->addConstraint("EqualValueConstraint", ct_base, params);
            }
            break;

            default:
              mooseError("Periodicity type not implemented");
          }
        }
      }
    }
}
