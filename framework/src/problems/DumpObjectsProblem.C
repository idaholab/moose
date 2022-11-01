//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DumpObjectsProblem.h"
#include "DumpObjectsNonlinearSystem.h"
#include "AuxiliarySystem.h"
#include <sstream>

#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", DumpObjectsProblem);

InputParameters
DumpObjectsProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addClassDescription("Single purpose problem object that does not run the given input but "
                             "allows deconstructing actions into their series of underlying Moose "
                             "objects and variables.");
  params.addRequiredParam<std::string>(
      "dump_path", "Syntax path of the action of which to dump the generated syntax");
  return params;
}

DumpObjectsProblem::DumpObjectsProblem(const InputParameters & parameters)
  : FEProblemBase(parameters), _nl_sys(std::make_shared<DumpObjectsNonlinearSystem>(*this, "nl0"))
{
  _nl[0] = _nl_sys;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");
  newAssemblyArray(_nl);

  // Create extra vectors and matrices if any
  createTagVectors();

  // Create extra solution vectors if any
  createTagSolutions();
}

std::string
DumpObjectsProblem::deduceNecessaryParameters(const std::string & type,
                                              const InputParameters & parameters)
{
  auto factory_params = stringifyParameters(_factory.getValidParams(type));
  auto specified_params = stringifyParameters(parameters);

  std::string param_text;
  for (auto & value_pair : specified_params)
  {
    // parameter name
    const auto & param_name = value_pair.first;
    const auto & param_value = value_pair.second;

    auto factory_it = factory_params.find(param_name);
    if (factory_it == factory_params.end() || factory_it->second != param_value)
      param_text += "    " + param_name + " = " + param_value + '\n';
  }

  return param_text;
}

void
DumpObjectsProblem::dumpObjectHelper(const std::string & system,
                                     const std::string & type,
                                     const std::string & name,
                                     const InputParameters & parameters)
{
  auto path = _app.actionWarehouse().getCurrentActionName();
  auto param_text = deduceNecessaryParameters(type, parameters);

  // clang-format off
  _generated_syntax[path][system] +=
        "  [./" + name + "]\n"
      + "    type = " + type + '\n'
      +      param_text
      + "  [../]\n";
  // clang-format on
}

void
DumpObjectsProblem::dumpVariableHelper(const std::string & system,
                                       const std::string & var_name,
                                       FEFamily family,
                                       Order order,
                                       Real scale_factor,
                                       const std::set<SubdomainID> * const active_subdomains)
{
  auto path = _app.actionWarehouse().getCurrentActionName();
  std::string param_text;

  if (active_subdomains)
  {
    std::string blocks;
    for (auto & subdomain_id : *active_subdomains)
    {
      auto subdomain_name = _mesh.getMesh().subdomain_name(subdomain_id);
      if (subdomain_name == "")
        subdomain_name = std::to_string(subdomain_id);

      if (!blocks.empty())
        blocks += ' ';

      blocks += subdomain_name;
    }

    if (active_subdomains->size() > 1)
      blocks = "'" + blocks + "'";

    param_text += "    blocks = " + blocks + '\n';
  }

  if (family != LAGRANGE)
    param_text += "    family = " + libMesh::Utility::enum_to_string<FEFamily>(family) + '\n';
  if (order != FIRST)
    param_text += "    order = " + libMesh::Utility::enum_to_string<Order>(order) + '\n';
  if (scale_factor != 1.0)
    param_text += "    scale = " + std::to_string(scale_factor);

  // clang-format off
  _generated_syntax[path][system] +=
        "  [./" + var_name + "]\n"
      +      param_text
      + "  [../]\n";
  // clang-format on
}

void
DumpObjectsProblem::solve(unsigned int)
{
  dumpGeneratedSyntax(getParam<std::string>("dump_path"));
}

void
DumpObjectsProblem::dumpGeneratedSyntax(const std::string path)
{
  auto pathit = _generated_syntax.find(path);
  if (pathit == _generated_syntax.end())
    return;

  Moose::out << "**START DUMP DATA**\n";
  for (const auto & system_pair : pathit->second)
    Moose::out << '[' << system_pair.first << "]\n" << system_pair.second << "[]\n\n";
  Moose::out << "**END DUMP DATA**\n";
  Moose::out << std::flush;
}

std::map<std::string, std::string>
DumpObjectsProblem::stringifyParameters(const InputParameters & parameters)
{
  std::map<std::string, std::string> parameter_map;

  std::string syntax;
  if (parameters.isParamValid("parser_syntax"))
    syntax = parameters.get<std::string>("parser_syntax");

  for (auto & value_pair : parameters)
  {
    // parameter name
    const auto & param_name = value_pair.first;

    if (!parameters.isPrivate(param_name) && parameters.isParamValid(param_name))
    {
      if (param_name == "control_tags")
      {
        // deal with the control tags. The current parser_syntax is automatically added to this. So
        // we can remove the parameter if that's all there is in it
      }
      else
      {
        // special treatment for some types

        // parameter value
        std::string param_value;
        if (parameters.have_parameter<bool>(param_name))
        {
          const bool & b = parameters.get<bool>(param_name);
          param_value = b ? "true" : "false";
        }
        else
        {
          std::stringstream ss;
          value_pair.second->print(ss);
          param_value = ss.str();
        }

        // delete trailing space
        if (!param_value.empty() && param_value.back() == ' ')
          param_value.pop_back();

        // add quotes if the parameter contains spaces or is empty
        if (param_value.find_first_of(" ") != std::string::npos || param_value.length() == 0)
          param_value = "'" + param_value + "'";

        parameter_map[param_name] = param_value;
      }
    }
  }

  return parameter_map;
}
