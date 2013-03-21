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
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

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

// class static initialization
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddVariableAction>()
{
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable (additional orders not listed are allowed)");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");

  params.addParamNamesToGroup("scaling", "Advanced");

  return params;
}

AddVariableAction::AddVariableAction(const std::string & name, InputParameters params) :
    Action(name, params)
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
  std::string var_name = getShortName();
  FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

  bool is_nl_variables_action = getAction() == "add_variable";

  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName> >("block");
  for (std::vector<SubdomainName>::iterator it = block_param.begin(); it != block_param.end(); ++it)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(*it);
    blocks.insert(blk_id);
  }

  Real scale_factor = isParamValid("scaling") ? getParam<Real>("scaling") : 1;
  bool scalar_var = false;                              // true if adding scalar variable

  if (is_nl_variables_action)
  {
    if (fe_type.family == SCALAR)
    {
      _problem->addScalarVariable(var_name, fe_type.order, scale_factor);
      scalar_var = true;
    }
    else
    {
      if (blocks.empty())
          _problem->addVariable(var_name, fe_type, scale_factor);
      else
        _problem->addVariable(var_name, fe_type, scale_factor, &blocks);
    }
  }
  else
  {
    if (fe_type.family == SCALAR)
    {
      _problem->addAuxScalarVariable(var_name, fe_type.order);
      scalar_var = true;
    }
    else if (blocks.empty())
      _problem->addAuxVariable(var_name, fe_type);
    else
      _problem->addAuxVariable(var_name, fe_type, &blocks);
  }

  // Set initial condition
  Real initial = getParam<Real>("initial_condition");
  if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
  {
    if (scalar_var)
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
