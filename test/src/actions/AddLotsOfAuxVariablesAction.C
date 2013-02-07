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

#include "AddLotsOfAuxVariablesAction.h"
#include "Parser.h"
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
const Real AddLotsOfAuxVariablesAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<AddLotsOfAuxVariablesAction>()
{
  MooseEnum families("LAGRANGE, MONOMIAL, HERMITE, SCALAR, HIERARCHIC, CLOUGH, XYZ, SZABAB, BERNSTEIN", "LAGRANGE");
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FORTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<unsigned int>("number", "The number of variables to add");
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");

  return params;
}


AddLotsOfAuxVariablesAction::AddLotsOfAuxVariablesAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddLotsOfAuxVariablesAction::act()
{
  unsigned int number = getParam<unsigned int>("number");
  for(unsigned int cur_num = 0; cur_num < number; cur_num++)
  {
    std::string var_name = getShortName() + Moose::stringify(cur_num);
    FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                   Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

    std::set<SubdomainID> blocks;
    std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName> >("block");
    for (std::vector<SubdomainName>::iterator it = block_param.begin(); it != block_param.end(); ++it)
    {
      SubdomainID blk_id = _problem->mesh().getSubdomainID(*it);
      blocks.insert(blk_id);
    }

    bool scalar_var = false;                              // true if adding scalar variable

    if (fe_type.family == SCALAR)
    {
      _problem->addAuxScalarVariable(var_name, fe_type.order);
      scalar_var = true;
    }
    else if (blocks.empty())
      _problem->addAuxVariable(var_name, fe_type);
    else
      _problem->addAuxVariable(var_name, fe_type, &blocks);

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
}
