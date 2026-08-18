//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactLineSearchBase.h"
#include "PetscSupport.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "FEProblem.h"
#include "MooseError.h"
#include "NonlinearSystem.h"
#include "libmesh/petsc_solver_exception.h"

registerMooseObjectAliased("ContactApp", ContactLineSearchBase, "ContactLineSearch");

InputParameters
ContactLineSearchBase::validParams()
{
  InputParameters params = LineSearch::validParams();
  params.addRequiredParam<Real>("contact_ltol",
                                "The linear tolerance to use when the contact set is changing.");
  params.addRequiredParam<bool>("affect_ltol",
                                "Whether to change the linear tolerance from the default value "
                                "when the contact set is changing");
  MooseEnum backing_line_search("basic bt l2 cp");
  params.addRequiredParam<MooseEnum>(
      "backing_line_search",
      backing_line_search,
      "The standard PETSc SNES line search type used as the backing algorithm.");
  return params;
}

ContactLineSearchBase::ContactLineSearchBase(const InputParameters & parameters)
  : LineSearch(parameters),
    _backing_petsc_line_search(nullptr),
    _contact_ltol(getParam<Real>("contact_ltol")),
    _affect_ltol(getParam<bool>("affect_ltol")),
    _backing_line_search(getParam<MooseEnum>("backing_line_search"))
{
  _solver = dynamic_cast<PetscNonlinearSolver<Real> *>(
      _fe_problem.getNonlinearSystem(_nl_sys_num).nonlinearSolver());
  if (!_solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
}

ContactLineSearchBase::~ContactLineSearchBase()
{
  if (_backing_petsc_line_search)
    PetscCallAbort(comm().get(), SNESLineSearchDestroy(&_backing_petsc_line_search));
}

void
ContactLineSearchBase::setupBackingLineSearch()
{
  if (!_backing_petsc_line_search)
  {
    LibmeshPetscCall(SNESLineSearchCreate(comm().get(), &_backing_petsc_line_search));
    // The MooseEnum values (basic/bt/l2/cp) are exactly the PETSc SNESLineSearch type strings
    LibmeshPetscCall(SNESLineSearchSetType(
        _backing_petsc_line_search, static_cast<std::string>(_backing_line_search).c_str()));
    // Distinct prefix so users can tune this object's native PETSc options (e.g.
    // -contact_backing_snes_linesearch_damping) without MOOSE re-exposing each one
    LibmeshPetscCall(
        SNESLineSearchAppendOptionsPrefix(_backing_petsc_line_search, "contact_backing_"));
    LibmeshPetscCall(SNESLineSearchSetFromOptions(_backing_petsc_line_search));
  }

  // PetscNonlinearSolver::clear() destroys and recreates its SNES between solves (e.g. once per
  // time step), so the backing line search's SNES must be rebound every time this is called, not
  // only when the object itself is first created here.
  LibmeshPetscCall(SNESLineSearchSetSNES(_backing_petsc_line_search, _solver->snes()));
}
