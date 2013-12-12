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

// Standard includes
#include <sstream>
#include <stdexcept>

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"

// class static initialization
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddVariableAction>()
{
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  // Define the general input options
  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable (additional orders not listed are allowed)");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");

  // Advanced input options
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParamNamesToGroup("scaling", "Advanced");

  return params;
}

AddVariableAction::AddVariableAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _scalar_var(_fe_type.family == SCALAR)
{
}

MooseEnum
AddVariableAction::getNonlinearVariableFamilies()
{
  return MooseEnum("LAGRANGE, MONOMIAL, HERMITE, SCALAR, HIERARCHIC, CLOUGH, XYZ, SZABAB, BERNSTEIN, L2_LAGRANGE, L2_HIERARCHIC", "LAGRANGE");
}

MooseEnum
AddVariableAction::getNonlinearVariableOrders()
{
  return MooseEnum("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST", true);
}

void
AddVariableAction::act()
{
  // Get necessary data for creating a variable
  std::string var_name = getShortName();
  std::set<SubdomainID> blocks = getSubdomainIDs();
  Real scale_factor = isParamValid("scaling") ? getParam<Real>("scaling") : 1;

  // Scalar variable
  if (_scalar_var)
    _problem->addScalarVariable(var_name, _fe_type.order, scale_factor);

  // Block restricted variable
  else if (blocks.empty())
    _problem->addVariable(var_name, _fe_type, scale_factor);

  // Non-block restricted variable
  else
    _problem->addVariable(var_name, _fe_type, scale_factor, &blocks);

  // Set the initial condition
  setInitialCondition();
}

void
AddVariableAction::setInitialCondition()
{
  // Variable name
  std::string var_name = getShortName();

  // Set initial condition
  Real initial = getParam<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
  {
    if (_scalar_var)
    {
      // built a ScalarConstantIC object
      InputParameters params = _factory.getValidParams("ScalarConstantIC");
      params.set<VariableName>("variable") = var_name;
      params.set<Real>("value") = initial;
      _problem->addInitialCondition("ScalarConstantIC", "ic", params);
    }
    else
    {
      // built a ConstantIC object
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = var_name;
      params.set<Real>("value") = initial;
      _problem->addInitialCondition("ConstantIC", "ic", params);
    }
  }
}

std::set<SubdomainID>
AddVariableAction::getSubdomainIDs()
{
  // Extract and return the block ids supplied in the input
  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName> >("block");
  for (std::vector<SubdomainName>::iterator it = block_param.begin(); it != block_param.end(); ++it)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(*it);
    blocks.insert(blk_id);
  }
  return blocks;
}
