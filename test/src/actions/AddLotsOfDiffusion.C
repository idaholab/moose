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

#include "AddLotsOfDiffusion.h"
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
validParams<AddLotsOfDiffusion>()
{
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  InputParameters params = validParams<AddVariableAction>();
  params.addRequiredParam<unsigned int>("number", "The number of variables to add");

  return params;
}

AddLotsOfDiffusion::AddLotsOfDiffusion(const InputParameters & params) : AddVariableAction(params)
{
}

void
AddLotsOfDiffusion::act()
{
  unsigned int number = getParam<unsigned int>("number");

  if (_current_task == "add_variable")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);
      addVariable(var_name);
    }
  }
  else if (_current_task == "add_kernel")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);

      InputParameters params = _factory.getValidParams("Diffusion");
      params.set<NonlinearVariableName>("variable") = var_name;
      _problem->addKernel("Diffusion", var_name, params);
    }
  }
  else if (_current_task == "add_bc")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);

      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<BoundaryName>>("boundary").push_back("left");
      params.set<Real>("value") = 0;

      _problem->addBoundaryCondition("DirichletBC", var_name + "_left", params);

      params.set<std::vector<BoundaryName>>("boundary")[0] = "right";
      params.set<Real>("value") = 1;

      _problem->addBoundaryCondition("DirichletBC", var_name + "_right", params);
    }
  }
}
