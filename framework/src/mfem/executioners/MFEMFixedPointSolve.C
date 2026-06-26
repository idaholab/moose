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
MFEMFixedPointSolve::saveAllValues(const bool primary)
{
  // if (_transformed_sys)
  //   saveVariableValues(primary);
  // savePostprocessorValues(primary);
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
MFEMFixedPointSolve::findTransformedSystem(const bool primary)
{
  // // Find the system for the transformed variables. They must all belong to the same system
  // const auto & transformed_vars = primary ? _transformed_vars : _secondary_transformed_variables;
  // if (!transformed_vars.empty())
  // {
  //   if (_problem.hasAuxiliaryVariable(transformed_vars[0]))
  //     _transformed_sys = &_aux;
  //   else
  //     _transformed_sys = &_solver_sys;
  // }

  // for (const auto & var_name : transformed_vars)
  //   if (!_transformed_sys->hasVariable(var_name))
  //   {
  //     if (primary)
  //       paramError("transformed_variables",
  //                  "Transformed variables must all belong to the same system. Auxiliary and each "
  //                  "solver system cannot be mixed");
  //     else
  //       mooseError("Secondary transformed variables must all belong to the same system. Auxiliary "
  //                  "and each solver system cannot be mixed");
  //   }

  // if (primary && _transformed_sys == &_aux)
  //   mooseInfo("Transformation of auxiliary variables is only supported for auxiliary variables "
  //             "that are only transferred from the child application");

  // if (_transformed_sys)
  //   _systems_to_copy_previous_solutions_for.insert(_transformed_sys);
}
