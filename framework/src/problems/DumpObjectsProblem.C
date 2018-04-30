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
#include <sstream>

#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", DumpObjectsProblem);

template <>
InputParameters
validParams<DumpObjectsProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  params.addClassDescription("Single purpose problem object that does not run the given input but "
                             "allows deconstructing actions into their series of underlying Moose "
                             "objects and variables.");
  params.addRequiredParam<std::string>(
      "dump_path", "Syntax path of the action of which to dump the generated syntax");
  return params;
}

DumpObjectsProblem::DumpObjectsProblem(const InputParameters & parameters) : FEProblemBase(parameters),
  _nl_sys(std::make_shared<DumpObjectsNonlinearSystem>(*this, "nl0"))
{
  _nl = _nl_sys;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");
  newAssemblyArray(*_nl_sys);
}

void
DumpObjectsProblem::addVariable(const std::string & var_name,
                                const FEType & type,
                                Real scale_factor,
                                const std::set<SubdomainID> * const active_subdomains)
{
  dumpVariableHelper(
      "Variables", var_name, type.family, type.order, scale_factor, active_subdomains);
  FEProblemBase::addVariable(var_name, type, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addScalarVariable(const std::string & var_name,
                                      Order order,
                                      Real scale_factor,
                                      const std::set<SubdomainID> * const active_subdomains)
{
  dumpVariableHelper("Variables", var_name, SCALAR, order, scale_factor, active_subdomains);
  FEProblemBase::addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addAuxVariable(const std::string & var_name,
                                   const FEType & type,
                                   const std::set<SubdomainID> * const active_subdomains)
{
  dumpVariableHelper("AuxVariables", var_name, type.family, type.order, 1.0, active_subdomains);
  FEProblemBase::addAuxVariable(var_name, type, active_subdomains);
}

void
DumpObjectsProblem::addAuxScalarVariable(const std::string & var_name,
                                         Order order,
                                         Real scale_factor,
                                         const std::set<SubdomainID> * const active_subdomains)
{
  dumpVariableHelper("AuxVariables", var_name, SCALAR, order, 1.0, active_subdomains);
  FEProblemBase::addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addFunction(std::string type,
                                const std::string & name,
                                InputParameters parameters)
{
  dumpObjectHelper("Functions", type, name, parameters);
  FEProblemBase::addFunction(type, name, parameters);
}

void
DumpObjectsProblem::addKernel(const std::string & type,
                              const std::string & name,
                              InputParameters parameters)
{
  dumpObjectHelper("Kernels", type, name, parameters);
  FEProblemBase::addKernel(type, name, parameters);
}

void
DumpObjectsProblem::addNodalKernel(const std::string & type,
                                   const std::string & name,
                                   InputParameters parameters)
{
  dumpObjectHelper("NodalKernel", type, name, parameters);
  FEProblemBase::addNodalKernel(type, name, parameters);
}

void
DumpObjectsProblem::addScalarKernel(const std::string & type,
                                    const std::string & name,
                                    InputParameters parameters)
{
  dumpObjectHelper("ScalarKernels", type, name, parameters);
  FEProblemBase::addScalarKernel(type, name, parameters);
}

void
DumpObjectsProblem::addBoundaryCondition(const std::string & type,
                                         const std::string & name,
                                         InputParameters parameters)
{
  dumpObjectHelper("BCs", type, name, parameters);
  FEProblemBase::addBoundaryCondition(type, name, parameters);
}

void
DumpObjectsProblem::addConstraint(const std::string & type,
                                  const std::string & name,
                                  InputParameters parameters)
{
  dumpObjectHelper("Constraints", type, name, parameters);
  FEProblemBase::addConstraint(type, name, parameters);
}

void
DumpObjectsProblem::addAuxKernel(const std::string & type,
                                 const std::string & name,
                                 InputParameters parameters)
{
  dumpObjectHelper("AuxKernels", type, name, parameters);
  FEProblemBase::addAuxKernel(type, name, parameters);
}

void
DumpObjectsProblem::addAuxScalarKernel(const std::string & type,
                                       const std::string & name,
                                       InputParameters parameters)
{
  dumpObjectHelper("AuxScalarKernels", type, name, parameters);
  FEProblemBase::addAuxScalarKernel(type, name, parameters);
}

void
DumpObjectsProblem::addDiracKernel(const std::string & type,
                                   const std::string & name,
                                   InputParameters parameters)
{
  dumpObjectHelper("DiracKernels", type, name, parameters);
  FEProblemBase::addDiracKernel(type, name, parameters);
}

void
DumpObjectsProblem::addDGKernel(const std::string & type,
                                const std::string & name,
                                InputParameters parameters)
{
  dumpObjectHelper("DGKernels", type, name, parameters);
  FEProblemBase::addDGKernel(type, name, parameters);
}

void
DumpObjectsProblem::addInterfaceKernel(const std::string & type,
                                       const std::string & name,
                                       InputParameters parameters)
{
  dumpObjectHelper("InterfaceKernels", type, name, parameters);
  FEProblemBase::addInterfaceKernel(type, name, parameters);
}

void
DumpObjectsProblem::addInitialCondition(const std::string & type,
                                        const std::string & name,
                                        InputParameters parameters)
{
  dumpObjectHelper("ICs", type, name, parameters);
  FEProblemBase::addInitialCondition(type, name, parameters);
}

void
DumpObjectsProblem::addMaterial(const std::string & type,
                                const std::string & name,
                                InputParameters parameters)
{
  dumpObjectHelper("Materials", type, name, parameters);
  FEProblemBase::addMaterial(type, name, parameters);
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
DumpObjectsProblem::solve()
{
  dumpGeneratedSyntax(getParam<std::string>("dump_path"));
}

void
DumpObjectsProblem::dumpGeneratedSyntax(const std::string path)
{
  auto pathit = _generated_syntax.find(path);
  if (pathit == _generated_syntax.end())
    return;

  for (const auto & system_pair : pathit->second)
    Moose::out << '[' << system_pair.first << "]\n" << system_pair.second << "[]\n\n";
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
        auto param_bool = dynamic_cast<InputParameters::Parameter<bool> *>(value_pair.second);

        // parameter value
        std::string param_value;
        if (param_bool)
          param_value = param_bool->get() ? "true" : "false";
        else
        {
          std::stringstream ss;
          value_pair.second->print(ss);
          param_value = ss.str();
        }

        // delete trailing space
        if (param_value.back() == ' ')
          param_value.pop_back();

        // add quotes if the parameter contains spaces
        if (param_value.find_first_of(" ") != std::string::npos)
          param_value = "'" + param_value + "'";

        parameter_map[param_name] = param_value;
      }
    }
  }

  return parameter_map;
}
