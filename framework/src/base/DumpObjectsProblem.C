/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "DumpObjectsProblem.h"
#include <sstream>

template <>
InputParameters
validParams<DumpObjectsProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addRequiredParam<std::string>(
      "dump_path", "Syntax path of the action of which to dump the generated syntax");
  return params;
}

DumpObjectsProblem::DumpObjectsProblem(const InputParameters & parameters) : FEProblem(parameters)
{
}

void
DumpObjectsProblem::addVariable(const std::string & var_name,
                                const FEType & type,
                                Real scale_factor,
                                const std::set<SubdomainID> * const active_subdomains)
{
  FEProblem::addVariable(var_name, type, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addScalarVariable(const std::string & var_name,
                                      Order order,
                                      Real scale_factor,
                                      const std::set<SubdomainID> * const active_subdomains)
{
  FEProblem::addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addAuxVariable(const std::string & var_name,
                                   const FEType & type,
                                   const std::set<SubdomainID> * const active_subdomains)
{
  FEProblem::addAuxVariable(var_name, type, active_subdomains);
}

void
DumpObjectsProblem::addAuxScalarVariable(const std::string & var_name,
                                         Order order,
                                         Real scale_factor,
                                         const std::set<SubdomainID> * const active_subdomains)
{
  FEProblem::addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DumpObjectsProblem::addFunction(std::string type,
                                const std::string & name,
                                InputParameters parameters)
{
  FEProblem::addFunction(type, name, parameters);
  dumpHelper("Functions", type, name, parameters);
}

void
DumpObjectsProblem::addKernel(const std::string & type,
                              const std::string & name,
                              InputParameters parameters)
{
  FEProblem::addKernel(type, name, parameters);
  dumpHelper("Kernels", type, name, parameters);
}

void
DumpObjectsProblem::addNodalKernel(const std::string & type,
                                   const std::string & name,
                                   InputParameters parameters)
{
  FEProblem::addNodalKernel(type, name, parameters);
  dumpHelper("NodalKernel", type, name, parameters);
}

void
DumpObjectsProblem::addScalarKernel(const std::string & type,
                                    const std::string & name,
                                    InputParameters parameters)
{
  FEProblem::addScalarKernel(type, name, parameters);
  dumpHelper("ScalarKernels", type, name, parameters);
}

void
DumpObjectsProblem::addBoundaryCondition(const std::string & type,
                                         const std::string & name,
                                         InputParameters parameters)
{
  FEProblem::addBoundaryCondition(type, name, parameters);
  dumpHelper("BCs", type, name, parameters);
}

void
DumpObjectsProblem::addConstraint(const std::string & type,
                                  const std::string & name,
                                  InputParameters parameters)
{
  FEProblem::addConstraint(type, name, parameters);
  dumpHelper("Constraints", type, name, parameters);
}

void
DumpObjectsProblem::addAuxKernel(const std::string & type,
                                 const std::string & name,
                                 InputParameters parameters)
{
  FEProblem::addAuxKernel(type, name, parameters);
  dumpHelper("AuxKernels", type, name, parameters);
}

void
DumpObjectsProblem::addAuxScalarKernel(const std::string & type,
                                       const std::string & name,
                                       InputParameters parameters)
{
  FEProblem::addAuxScalarKernel(type, name, parameters);
  dumpHelper("AuxScalarKernels", type, name, parameters);
}

void
DumpObjectsProblem::addDiracKernel(const std::string & type,
                                   const std::string & name,
                                   InputParameters parameters)
{
  FEProblem::addDiracKernel(type, name, parameters);
  dumpHelper("DiracKernels", type, name, parameters);
}

void
DumpObjectsProblem::addDGKernel(const std::string & type,
                                const std::string & name,
                                InputParameters parameters)
{
  FEProblem::addDGKernel(type, name, parameters);
  dumpHelper("DGKernels", type, name, parameters);
}

void
DumpObjectsProblem::addInterfaceKernel(const std::string & type,
                                       const std::string & name,
                                       InputParameters parameters)
{
  FEProblem::addInterfaceKernel(type, name, parameters);
  dumpHelper("InterfaceKernels", type, name, parameters);
}

void
DumpObjectsProblem::addInitialCondition(const std::string & type,
                                        const std::string & name,
                                        InputParameters parameters)
{
  FEProblem::addInitialCondition(type, name, parameters);
  dumpHelper("ICs", type, name, parameters);
}

void
DumpObjectsProblem::addMaterial(const std::string & type,
                                const std::string & name,
                                InputParameters parameters)
{
  FEProblem::addMaterial(type, name, parameters);
  dumpHelper("Materials", type, name, parameters);
}

void
DumpObjectsProblem::dumpHelper(const std::string & system,
                               const std::string & type,
                               const std::string & name,
                               InputParameters parameters)
{
  auto path = _app.actionWarehouse().getCurrentActionName();

  std::string syntax;
  if (isParamValid("parser_syntax"))
    syntax = parameters.get<std::string>("parser_syntax");

  std::string param_text;
  for (auto & value_pair : parameters)
  {
    // parameter name
    const auto & param_name = value_pair.first;

    if (parameters.isParamSetByUser(param_name) && !parameters.isPrivate(param_name))
    {
      if (param_name == "control_tags")
      {
      }
      else
      {
        // parameter value
        std::stringstream ss;
        value_pair.second->print(ss);
        auto param_value = ss.str();

        // delete trailing space
        if (param_value.back() == ' ')
          param_value.pop_back();

        // add quotes if the parameter contains spaces
        if (param_value.find_first_of(" ") != std::string::npos)
          param_value = "'" + param_value + "'";

        param_text += "    " + param_name + " = " + param_value + '\n';
      }
    }
  }

  // clang-format off
  _generated_syntax[path][system] +=
        "  [./" + name + "]\n"
      + "    type = " + type + '\n'
      +      param_text
      + "  [../]\n";
  // clang-format on
}

void
DumpObjectsProblem::initialSetup()
{
  dumpGeneratedSyntax(getParam<std::string>("dump_path"));
  std::abort();
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
