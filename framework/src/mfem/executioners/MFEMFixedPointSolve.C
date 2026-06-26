//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MFEMFixedPointSolve.h"

#include "FEProblem.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"
#include "EigenExecutionerBase.h"
#include "Convergence.h"
#include "ConvergenceIterationTypes.h"
#include "MooseUtils.h"

MFEMFixedPointSolve::MFEMFixedPointSolve(Executioner & ex) : FixedPointSolve(ex) {}

void
MFEMFixedPointSolve::initialSetup()
{
  FixedPointSolve::initialSetup();

  // // Add to the systems to copy if requested in the Problem
  // for (const auto i : make_range(_problem.numSolverSystems()))
  //   if (_problem.needsPreviousMultiAppFixedPointIterationSolution(i))
  //     _systems_to_copy_previous_solutions_for.insert(&_problem.getSolverSystem(i));
  // if (_problem.needsPreviousMultiAppFixedPointIterationAuxiliary())
  //   _systems_to_copy_previous_solutions_for.insert(&_aux);
}

void
MFEMFixedPointSolve::copyPreviousFixedPointSolutions()
{
  // Save the previous fixed point iteration solution and aux variables if requested
  // for (auto * sys : _systems_to_copy_previous_solutions_for)
  //   sys->copyPreviousFixedPointSolutions();
}

void
MFEMFixedPointSolve::updateVariableDoFsForTransform(
    const std::vector<std::string> & transformed_var_names, const bool primary)
{
  // if ((_relax_factor != 1.0 || !dynamic_cast<PicardSolve *>(this)) &&
  //     transformed_var_names.size() > 0)
  // {
  //   // Snag all of the local dof indices for all of these variables
  //   AllLocalDofIndicesThread aldit(_problem, transformed_var_names);
  //   libMesh::ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
  //   Threads::parallel_reduce(elem_range, aldit);

  //   if (primary)
  //   {
  //     _transformed_dofs = aldit.getDofIndices();
  //   }
  //   else
  //   {
  //     _secondary_transformed_dofs = aldit.getDofIndices();
  //   }
  // }
}

void
MFEMFixedPointSolve::transformVariables(const bool primary)
{
}

void
MFEMFixedPointSolve::transformPostprocessors(const bool primary)
{
}

void
MFEMFixedPointSolve::saveVariableValues(const bool primary)
{
}

void
MFEMFixedPointSolve::savePostprocessorValues(const bool primary)
{
}

void
MFEMFixedPointSolve::saveAllValues(const bool primary)
{
  // if (_transformed_sys)
  //   saveVariableValues(primary);
  // savePostprocessorValues(primary);
}
