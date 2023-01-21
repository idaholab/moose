//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BasicExternalProblem.h"
#include "AuxiliarySystem.h"

registerMooseObject("MooseTestApp", BasicExternalProblem);

InputParameters
BasicExternalProblem::validParams()
{
  return ExternalProblem::validParams();
}

BasicExternalProblem::BasicExternalProblem(const InputParameters & params) : ExternalProblem(params)
{
}

void
BasicExternalProblem::addExternalVariables()
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "MONOMIAL";
  var_params.set<MooseEnum>("order") = "CONSTANT";

  addAuxVariable("MooseVariable", "heat_source", var_params);
  _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();
}

void
BasicExternalProblem::externalSolve()
{
}

void
BasicExternalProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
      break;
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      auto & solution = _aux->solution();
      const auto sys_number = _aux->number();
      const auto & mesh = _mesh.getMesh();

      for (const auto elem_ptr : mesh.element_ptr_range())
      {
        auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
        solution.set(dof_idx, 12345);
      }

      // close the parallel solution
      solution.close();
      // Make sure to update the ghosted current_local_solution (from the parallel solution) which
      // is the thing everyone actually uses
      _aux->system().update();
      break;
    }
    default:
      mooseError("bad!");
  }
}
