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


#include "libmesh/libmesh_config.h"

#if LIBMESH_HAVE_SLEPC

// moose includes
#include "NonlinearEigenSystem.h"
#include "FEProblem.h"
#include "TimeIntegrator.h"

// libmesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"


NonlinearEigenSystem::NonlinearEigenSystem(FEProblem & fe_problem, const std::string & name) :
  NonlinearSystemBase(fe_problem, fe_problem.es().add_system<TransientEigenSystem>(name), name),
  _transient_sys(fe_problem.es().get_system<TransientEigenSystem>(name))
{

}

NonlinearEigenSystem::~NonlinearEigenSystem()
{
}


void
NonlinearEigenSystem::solve()
{
  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;
  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();
  _time_integrator->solve();
  _time_integrator->postSolve();
}


void
NonlinearEigenSystem::stopSolve()
{
  mooseError("did not implement yet \n");
}



void
NonlinearEigenSystem::setupFiniteDifferencedPreconditioner()
{
  mooseError("did not implement yet \n");
}


bool
NonlinearEigenSystem::converged()
{
  mooseError("did not implement yet \n");
  return false;
}

unsigned int
NonlinearEigenSystem::getCurrentNonlinearIterationNumber()
{
  mooseError("did not implement yet \n");
  return 0;
}


 NumericVector<Number> &
 NonlinearEigenSystem::RHS()
{
  mooseError("did not implement yet \n");
  //return NULL;
}


NonlinearSolver<Number> *
NonlinearEigenSystem::nonlinearSolver()
{
  mooseError("did not implement yet \n");
  return NULL;
}


#endif /* LIBMESH_HAVE_SLEPC */
