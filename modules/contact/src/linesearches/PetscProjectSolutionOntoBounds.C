//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscProjectSolutionOntoBounds.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "GeometricSearchData.h"
#include "PenetrationLocator.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_solver_exception.h"
#include "libmesh/petsc_vector.h"
#include <petscdm.h>

registerMooseObject("ContactApp", PetscProjectSolutionOntoBounds);

InputParameters
PetscProjectSolutionOntoBounds::validParams()
{
  return LineSearch::validParams();
}

PetscProjectSolutionOntoBounds::PetscProjectSolutionOntoBounds(const InputParameters & parameters)
  : LineSearch(parameters), _nl(_fe_problem.getNonlinearSystemBase())
{
  _solver = dynamic_cast<PetscNonlinearSolver<Real> *>(
      _fe_problem.getNonlinearSystem().nonlinearSolver());
  if (!_solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
}

void
PetscProjectSolutionOntoBounds::initialSetup()
{
  _displaced_problem = _fe_problem.getDisplacedProblem().get();
  if (!_displaced_problem)
    mooseError("This line search only makes sense in a displaced context");
}

void
PetscProjectSolutionOntoBounds::lineSearch()
{
  PetscBool changed_y = PETSC_FALSE;
  PetscErrorCode ierr;
  Vec X, F, Y, W, G;
  SNESLineSearch line_search;
  PetscReal fnorm, xnorm, ynorm;
  PetscBool domainerror;
  SNES snes = _solver->snes();

  ierr = SNESGetLineSearch(snes, &line_search);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetNorms(line_search, &xnorm, &fnorm, &ynorm);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED);
  LIBMESH_CHKERR(ierr);

  ierr = SNESLineSearchPreCheck(line_search, X, Y, &changed_y);
  LIBMESH_CHKERR(ierr);

  // apply the full newton step
  ierr = VecWAXPY(W, -1., Y, X);
  LIBMESH_CHKERR(ierr);

  {
    PetscVector<Number> solution(W, this->comm());

    _nl.setSolution(solution);

    // Displace the mesh and update the displaced geometric search objects
    _displaced_problem->updateMesh();
  }

  // A reference to the displaced MeshBase object
  const auto & mesh = _displaced_problem->mesh().getMesh();

  const auto & pen_locs = _displaced_problem->geomSearchData().getPenetrationLocators();

  // Keep track of the secondary nodes that we push back onto the primary face. We'll eventually
  // check to make sure that we didn't have a corner node at the intersection of two secondary faces
  // that we tried to displace twice. As this stands now this won't cover the case wherethe
  // intersection happens only across processes
  std::set<dof_id_type> nodes_displaced;

  std::vector<unsigned int> disp_nums;

  // Generate the displaced variable numbers
  for (const auto & disp_name : _displaced_problem->getDisplacementVarNames())
    disp_nums.push_back(_nl.system().variable_number(disp_name));

  for (const auto & pen_loc : pen_locs)
    for (const auto & pinfo_pair : pen_loc.second->_penetration_info)
    {
      auto node_id = pinfo_pair.first;
      auto pen_info = pinfo_pair.second;

      // We have penetration
      if (pen_info->_distance > 0)
      {
// Avoid warning in optimized modes about unused variables
#ifndef NDEBUG
        // Make sure we haven't done this node before
        auto pair = nodes_displaced.insert(node_id);
        mooseAssert(pair.second, "Node id " << node_id << " has already been displaced");
#endif

        const auto & node = mesh.node_ref(node_id);

        // If this is not a local node, we will let displacement happen on another process
        if (node.processor_id() != this->processor_id())
          continue;

        // The vector that we need to displace by
        auto required_solution_change = pen_info->_distance * pen_info->_normal;

        unsigned component = 0;
        std::vector<PetscInt> indices;
        std::vector<PetscScalar> values;
        for (auto disp_num : disp_nums)
        {
          auto dof_number = node.dof_number(/*sys=*/0, disp_num, /*component=*/0);
          indices.push_back(static_cast<PetscInt>(dof_number));
          values.push_back(static_cast<PetscScalar>(required_solution_change(component++)));
        }
        ierr = VecSetValues(
            W, static_cast<PetscInt>(indices.size()), indices.data(), values.data(), ADD_VALUES);
        LIBMESH_CHKERR(ierr);
      }
    }

  ierr = VecAssemblyBegin(W);
  LIBMESH_CHKERR(ierr);
  ierr = VecAssemblyEnd(W);
  LIBMESH_CHKERR(ierr);

  ierr = SNESComputeFunction(snes, W, F);
  LIBMESH_CHKERR(ierr);
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  LIBMESH_CHKERR(ierr);
  if (domainerror)
  {
    ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN);
    LIBMESH_CHKERR(ierr);
  }
  ierr = VecNorm(F, NORM_2, &fnorm);
  LIBMESH_CHKERR(ierr);

  ierr = VecCopy(W, X);
  LIBMESH_CHKERR(ierr);

  ierr = SNESLineSearchComputeNorms(line_search);
  LIBMESH_CHKERR(ierr);
}
