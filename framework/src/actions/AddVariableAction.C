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

#include "AddVariableAction.h"
#include "Parser.h"
#include "MProblem.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"

// class static initializiation
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddVariableAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<std::vector<unsigned int> >("block", "The block id where this variable lives");

  return params;
}


AddVariableAction::AddVariableAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _variable_to_read(""),
    _timestep_to_read(2)
  
{
}

void
AddVariableAction::act()
{
  std::string var_name = getShortName();
  bool is_variables_block = Parser::pathContains(_name, "Variables");
  if (is_variables_block)
  {
    std::set<subdomain_id_type> blocks;
    std::vector<unsigned int> block_param = getParam<std::vector<unsigned int> >("block");
    for (std::vector<unsigned int>::iterator it = block_param.begin(); it != block_param.end(); ++it)
      blocks.insert(*it);
    Real scale_factor = getParam<Real>("scaling");

    if (blocks.empty())
    {
      _problem->addVariable(var_name,
                           FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                  Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
                           scale_factor);
    }
    else
      _problem->addVariable(var_name,
                           FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                  Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
                           scale_factor,
                           &blocks);
  }
  else
  {
    _problem->addAuxVariable(var_name,
                            FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
                                   Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))));
  }

  // Set initial condition
  Real initial = getParam<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
    _problem->addInitialCondition(var_name, initial);
}
