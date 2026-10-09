//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBlockDiagonalPreconditioner.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMBlockDiagonalPreconditioner);

namespace Moose::MFEM
{

void
BlockDiagonalPreconditioner::SetOperator(const mfem::Operator & op)
{
  height = op.Height();
  width = op.Width();

  // cast the incoming operator into a BlockOperator. If it comes out null,
  // then we have a single-variable system
  const auto * const block_op = dynamic_cast<const mfem::BlockOperator *>(&op);

  // mfem::BlockDiagonalPreconditioner references _offsets, so release it before they change
  _block_diag_precon.reset();
  if (block_op)
    _offsets = block_op->RowOffsets();
  else
  {
    // Fallback for using BlockDiagonalPreconditioner with a single variable
    _offsets.SetSize(2);
    _offsets[0] = 0;
    _offsets[1] = op.Height();
  }

  mooseAssert(static_cast<std::size_t>(_offsets.Size() - 1) == _block_solvers.size(),
              "Number of block solvers does not match the number of diagonal blocks.");

  _block_diag_precon = std::make_unique<mfem::BlockDiagonalPreconditioner>(_offsets);
  for (const auto i : index_range(_block_solvers))
  {
    const mfem::Operator & block =
        block_op ? block_op->GetBlock(i, i) : op; // just use op if we have single-variable system
    auto & block_solver = *_block_solvers[i];
    block_solver.SetOperator(const_cast<mfem::Operator &>(block));
    _block_diag_precon->SetDiagonalBlock(
        i,
        &block_solver
             .GetSolver()); // LinearSolverBase::GetSolver returns ref to underlying mfem object
  }
}

void
BlockDiagonalPreconditioner::Mult(const mfem::Vector & x, mfem::Vector & y) const
{
  mooseAssert(_block_diag_precon,
              "BlockDiagonalPreconditioner was not given an operator before use.");
  _block_diag_precon->Mult(x, y);
}

} // namespace Moose::MFEM

InputParameters
MFEMBlockDiagonalPreconditioner::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Block-diagonal preconditioner for multi-variable MFEM equation "
                             "systems, applying a separate preconditioner to the diagonal block "
                             "of each trial variable.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "Trial variables whose diagonal blocks are preconditioned.");
  params.addRequiredParam<std::vector<MFEMSolverName>>(
      "preconditioners", "Preconditioner for the diagonal block of each entry in 'variables'.");
  return params;
}

MFEMBlockDiagonalPreconditioner::MFEMBlockDiagonalPreconditioner(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    _variables(getParam<std::vector<VariableName>>("variables"))
{
  const auto & names = getParam<std::vector<MFEMSolverName>>("preconditioners");
  if (names.size() != _variables.size())
    paramError("preconditioners", "Must have one entry for each entry in 'variables'.");

  // use std::set to check for duplicates
  if (std::set<MFEMSolverName>(names.begin(), names.end()).size() != names.size())
    paramError("preconditioners", "Each diagonal block requires its own preconditioner object.");

  for (const auto & name : names)
  {
    auto & precon = getMFEMProblem().getMFEMObject<Moose::MFEM::LinearSolverBase>(
        "Moose::MFEM::SolverBase", name);
    // take over shared ownership so the sub-solvers outlive this preconditioner
    _block_preconditioners.push_back(
        std::static_pointer_cast<Moose::MFEM::LinearSolverBase>(precon.shared_from_this()));
  }

  ConstructSolver();
}

void
MFEMBlockDiagonalPreconditioner::ConstructSolver()
{
  // Makes sure that GetSolver() returns the BlockDiagonalPreconditioner wrapper object
  _solver = std::make_unique<Moose::MFEM::BlockDiagonalPreconditioner>();
}

void
MFEMBlockDiagonalPreconditioner::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();

  // Trial var names are only known once the equation system is initialised,
  // so the sub-solvers are ordered by block index now, instead of at
  // time of construction.
  const auto & trial_var_names = _equation_system->GetTrialVarNames();
  if (trial_var_names.size() != _variables.size())
    paramError("variables", "Must list each trial variable of the equation system exactly once.");

  std::vector<Moose::MFEM::LinearSolverBase *> block_solvers(trial_var_names.size(), nullptr);
  for (const auto i : index_range(_variables))
  {
    const auto it = std::find(trial_var_names.begin(), trial_var_names.end(), _variables[i]);
    if (it == trial_var_names.end())
      paramError("variables",
                 "Variable '",
                 _variables[i],
                 "' is not a trial variable of the equation system.");
    auto & block_solver = block_solvers[std::distance(trial_var_names.begin(), it)];

    // check if we have already assigned this entry
    if (block_solver)
      paramError("variables", "Variable '", _variables[i], "' is listed more than once.");
    block_solver = _block_preconditioners[i].get();
  }

  cast_ref<Moose::MFEM::BlockDiagonalPreconditioner &>(GetSolver())
      .SetBlockSolvers(std::move(block_solvers));
}

#endif
