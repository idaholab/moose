//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseStaticCondensationPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseUtils.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/implicit_system.h"
#include "libmesh/static_condensation_dof_map.h"

registerMooseObjectAliased("MooseApp", MooseStaticCondensationPreconditioner, "StaticCondensation");

InputParameters
MooseStaticCondensationPreconditioner::validParams()
{
  InputParameters params = SingleMatrixPreconditioner::validParams();
  params.addClassDescription("Static condensation preconditioner");
  params.set<bool>("full") = true;
  params.suppressParameter<bool>("full");
  params.addParam<std::vector<NonlinearVariableName>>(
      "dont_condense_vars",
      {},
      "A list of variables for whom to not statically condense their degrees of freedom out of the "
      "system. By default all degrees of freedom on element interiors are condensed out.");
  return params;
}

std::string
MooseStaticCondensationPreconditioner::prefix() const
{
  // We always prefix the condensed system with the nonlinear system name regardless of the number
  // of systems in the problem. Maybe we'll change this later for more consistency?
  return "-" + petscPrefix();
}

std::string
MooseStaticCondensationPreconditioner::petscPrefix() const
{
  // We always prefix the condensed system with the nonlinear system name regardless of the number
  // of systems in the problem. Maybe we'll change this later for more consistency?
  return _nl.name() + "_condensed_";
}

MooseStaticCondensationPreconditioner::MooseStaticCondensationPreconditioner(
    const InputParameters & params)
  : SingleMatrixPreconditioner(params)
{
  if (_fe_problem.solverParams(_nl.number())._type != Moose::ST_PJFNK)
    mooseError(
        "Static condensation preconditioning should use a PJFNK solve type. This is because it "
        "provides a means to compute the action of the Jacobian on vectors, which we otherwise "
        "would not have because when using static condensation, the Jacobian is never formed. Note "
        "that actions of the Jacobian on a vector are necessary for things like: GMRES, printing "
        "of linear residuals, or application of line searches like cubit backtracking. These "
        "particular operations can be avoided by using '-ksp_type preonly', disabling printing of "
        "linear residuals, and using the 'cp' or 'basic' line search.");

  const auto & root_old_prefix = _nl.prefix();
  const std::string root_new_prefix = prefix();
  auto change_prefix = [this, &root_old_prefix, &root_new_prefix](const std::string & petsc_system)
  {
    Moose::PetscSupport::changePetscOptionsPrefix(
        _fe_problem, root_old_prefix + petsc_system + "_", root_new_prefix + petsc_system + "_");
  };
  change_prefix("ksp");
  change_prefix("pc");

  auto * const implicit_sys = dynamic_cast<libMesh::ImplicitSystem *>(&_nl.system());
  if (!implicit_sys)
    mooseError("Static condensation can only be used with implicit systems");
  implicit_sys->create_static_condensation();
  _sc_dof_map = &implicit_sys->get_static_condensation_dof_map();
  _sc_system_matrix = &implicit_sys->get_static_condensation_system_matrix();
  std::unordered_set<unsigned int> uncondensed_vars;
  for (auto & nl_var_name : getParam<std::vector<NonlinearVariableName>>("dont_condense_vars"))
    uncondensed_vars.insert(_nl.getVariable(0, nl_var_name).number());
  _sc_dof_map->dont_condense_vars(uncondensed_vars);
}
