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
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"
#include "fe.h"

// class static initialization
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddVariableAction>()
{
  MooseEnum families("LAGRANGE, MONOMIAL, HERMITE, SCALAR, HIERARCHIC, CLOUGH, XYZ, SZABAB, BERNSTEIN", "LAGRANGE");
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FORTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable");
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

void
AddVariableAction::act()
{
  std::string var_name = getShortName();
  FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
  bool is_variables_block = Parser::pathContains(_name, "Variables");

  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName> >("block");
  for (std::vector<SubdomainName>::iterator it = block_param.begin(); it != block_param.end(); ++it)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(*it);
    blocks.insert(blk_id);
  }

  Real scale_factor = getParam<Real>("scaling");
  bool scalar_var = false;                              // true if adding scalar variable

  if (is_variables_block)
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
    if (scale_factor != 1.0)
      mooseWarning("Currently the Auxiliary System ignores scaling factors - please contact the MOOSE team");

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
      InputParameters params = Factory::instance()->getValidParams("ScalarConstantIC");
      params.set<VariableName>("variable") = var_name;
      params.set<Real>("value") = initial;
      _problem->addInitialCondition("ScalarConstantIC", "ic", params);
    }
    else
    {
      // built a ConstantIC object
      InputParameters params = Factory::instance()->getValidParams("ConstantIC");
      params.set<VariableName>("variable") = var_name;
      params.set<Real>("value") = initial;
      _problem->addInitialCondition("ConstantIC", "ic", params);
    }
  }
}
