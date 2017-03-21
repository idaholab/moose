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

#include "AddMatAndKernel.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "DirichletBC.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddMatAndKernel>()
{
  InputParameters params = validParams<AddVariableAction>();
  return params;
}

AddMatAndKernel::AddMatAndKernel(const InputParameters & params) : AddVariableAction(params) {}

void
AddMatAndKernel::act()
{
  std::string var_name = "var1";
  if (_current_task == "add_variable")
    addVariable(var_name);
  else if (_current_task == "add_kernel")
  {
    InputParameters params = _factory.getValidParams("MatDiffusion");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<MaterialPropertyName>("prop_name") = "prop1";
    _problem->addKernel("MatDiffusion", "kern1", params);
  }
  else if (_current_task == "add_material")
  {
    InputParameters params = _factory.getValidParams("GenericConstantMaterial");
    params.set<std::vector<std::string>>("prop_names") = {"prop1"};
    params.set<std::vector<Real>>("prop_values") = {42};
    _problem->addMaterial("GenericConstantMaterial", "mat1", params);
  }
}
