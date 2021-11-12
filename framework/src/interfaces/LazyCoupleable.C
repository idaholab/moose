//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LazyCoupleable.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseApp.h"
#include "Executioner.h"

#include "libmesh/simple_range.h"

LazyCoupleable::LazyCoupleable(const MooseObject * moose_object)
  : _l_parameters(moose_object->parameters()),
    _l_name(_l_parameters.get<std::string>("_object_name")),
    _l_fe_problem(nullptr),
    _l_app(moose_object->getMooseApp())
{
  for (const auto & var_name :
       as_range(std::make_pair(_l_parameters.coupledVarsBegin(), _l_parameters.coupledVarsEnd())))
    _coupled_var_numbers[var_name] = std::make_unique<unsigned int>(0);
}

void
LazyCoupleable::setFEProblemPtr(FEProblemBase * fe_problem)
{
  _l_fe_problem = fe_problem;
  init();
}

void
LazyCoupleable::init()
{
  for (const auto & var_pair : _coupled_var_numbers)
  {
    if (!_l_fe_problem->hasVariable(var_pair.first))
      mooseError("Unable to find variable ", var_pair.first);

    auto & moose_var = _l_fe_problem->getVariable(
        0, var_pair.first, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
    if (moose_var.kind() == Moose::VAR_NONLINEAR)
      *(var_pair.second) = moose_var.number();
    else
      *(var_pair.second) = std::numeric_limits<unsigned int>::max() - moose_var.number();
  }
}

unsigned int &
LazyCoupleable::coupled(const std::string & var_name, unsigned int /*comp*/)
{
  if (!_l_fe_problem)
  {
    auto executioner_ptr = _l_app.getExecutioner();
    if (!executioner_ptr)
      mooseError("Executioner is nullptr in LazyCoupleable. You cannot call the \"coupled\" method "
                 "until the add_algebraic_rm task");
    setFEProblemPtr(&executioner_ptr->feProblem());
  }

  const auto & var_pair = _coupled_var_numbers.find(var_name);
  mooseAssert(var_pair != _coupled_var_numbers.end(), "Internal error in LazyCoupleable");

  return *(var_pair->second);
}
