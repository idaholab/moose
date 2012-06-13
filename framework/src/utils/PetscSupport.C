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

#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

#include "Moose.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"

//libMesh Includes
#include "libmesh_common.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "linear_implicit_system.h"
#include "sparse_matrix.h"
#include "petsc_vector.h"
#include "petsc_matrix.h"
#include "petsc_linear_solver.h"
#include "petsc_preconditioner.h"
#include "getpot.h"

//PETSc includes
#include <petsc.h>
#include <petscsnes.h>
#include <petscksp.h>

#if PETSC_VERSION_LESS_THAN(3,3,0)
// PETSc 3.2.x and lower
#include <private/kspimpl.h>
#include <private/snesimpl.h>
#else
// PETSc 3.3.0+
#include <petsc-private/kspimpl.h>
#include <petsc-private/snesimpl.h>
#endif

namespace Moose
{
namespace PetscSupport
{

void petscSetOptions(const std::vector<std::string> & petsc_options,
                     const std::vector<std::string> & petsc_options_inames,
                     const std::vector<std::string> & petsc_options_values)
{
  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("Petsc names and options are not the same length");

  for (unsigned int i=0; i<petsc_options.size(); ++i)
    PetscOptionsSetValue(petsc_options[i].c_str(), PETSC_NULL);

  for (unsigned int i=0; i<petsc_options_inames.size(); ++i)
    PetscOptionsSetValue(petsc_options_inames[i].c_str(), petsc_options_values[i].c_str());
}

PetscErrorCode  petscConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *dummy)
{
  NonlinearSystem *system = (NonlinearSystem *) dummy;      // C strikes

  *reason = KSP_CONVERGED_ITERATING;

  //If it's the beginning of a new set of iterations, reset last_rnorm
  if (!n)
    system->_last_rnorm = 1e99;

  PetscReal norm_diff = std::fabs(rnorm - system->_last_rnorm);

  if(norm_diff < system->_l_abs_step_tol)
  {
    *reason = KSP_CONVERGED_RTOL;
    return(0);
  }

  system->_last_rnorm = rnorm;

  // From here, we want the default behavior of the KSPDefaultConverged
  // test, but we don't want PETSc to die in that function with a
  // CHKERRQ call... therefore we Push/Pop a different error handler
  // and then call KSPDefaultConverged().  Finally, if we hit the
  // max iteration count, we want to set KSP_CONVERGED_ITS.
  PetscPushErrorHandler(PetscReturnErrorHandler,/* void* ctx= */PETSC_NULL);

  // As of PETSc 3.0.0, you must call KSPDefaultConverged with a
  // non-NULL context pointer which must be created with
  // KSPDefaultConvergedCreate(), and destroyed with
  // KSPDefaultConvergedDestroy().
  /*PetscErrorCode ierr = */
  KSPDefaultConverged(ksp, n, rnorm, reason, dummy);

  // Pop the Error handler we pushed on the stack to go back
  // to default PETSc error handling behavior.
  PetscPopErrorHandler();

  // If we hit max its then we consider that converged
  if (n >= ksp->max_it) *reason = KSP_CONVERGED_ITS;

  if(*reason == KSP_CONVERGED_ITS || *reason == KSP_CONVERGED_RTOL)
    system->_current_l_its.push_back(n);

  return 0;
}

PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void * dummy)
{
  NonlinearSystem *system = (NonlinearSystem *) dummy;      // C strikes

//  std::cout<<"Is converged!"<<std::endl;

  // unused
  // TransientNonlinearImplicitSystem * system = dynamic_cast<TransientNonlinearImplicitSystem *>(&_equation_system->get_system("NonlinearSystem"));

  //  for(unsigned int var = 0; var < system->n_vars(); var++)
  //    std::cout<<var<<": "<<system->calculate_norm(*system->rhs,var,DISCRETE_L2)<<std::endl;

  *reason = SNES_CONVERGED_ITERATING;

  if (!it)
  {
    /* set parameter for default relative tolerance convergence test */
    /* _initial_residual has already been computed by the NonlinearSystem at this point */

//    snes->ttol = fnorm*snes->rtol;
//    system->_initial_residual = fnorm;
    snes->ttol = system->_initial_residual*snes->rtol;
    system->_last_nl_rnorm = system->_initial_residual;
  }
  if (fnorm != fnorm)
  {
    PetscInfo(snes,"Failed to converge, function norm is NaN\n");
    *reason = SNES_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < snes->abstol)
  {
    PetscInfo2(snes,"Converged due to function norm %G < %G\n",fnorm,snes->abstol);
    *reason = SNES_CONVERGED_FNORM_ABS;
  }
  else if (snes->nfuncs >= snes->max_funcs)
  {
    PetscInfo2(snes,"Exceeded maximum number of function evaluations: %D > %D\n",snes->nfuncs,snes->max_funcs);
    *reason = SNES_DIVERGED_FUNCTION_COUNT;
  }
  else if(it &&
          snes->rtol > system->_last_nl_rnorm &&
          fnorm >= system->_initial_residual * (1.0/snes->rtol))
  {
    PetscInfo2(snes,"Nonlinear solve was blowing up!",snes->nfuncs,snes->max_funcs);
#if PETSC_VERSION_LESS_THAN(3,2,0)
    *reason = SNES_DIVERGED_LS_FAILURE;
#else
    // PETSc 3.2.0 +
    *reason = SNES_DIVERGED_LINE_SEARCH;
#endif
  }

  if (it && !*reason)
  {
    if (fnorm <= snes->ttol)
    {
      PetscInfo2(snes,"Converged due to function norm %G < %G (relative tolerance)\n",fnorm,snes->ttol);
      *reason = SNES_CONVERGED_FNORM_RELATIVE;
    }
#if PETSC_VERSION_LESS_THAN(3,3,0)
    // PETSc 3.2.x-
    else if (pnorm < snes->xtol*xnorm)
    {
      PetscInfo3(snes,"Converged due to small update length: %G < %G * %G\n",pnorm,snes->xtol,xnorm);
      *reason = SNES_CONVERGED_PNORM_RELATIVE;
    }
#else
    // PETSc 3.3.0+
    else if (pnorm < snes->stol*xnorm)
    {
      PetscInfo3(snes,"Converged due to small update length: %G < %G * %G\n",pnorm,snes->stol,xnorm);
      *reason = SNES_CONVERGED_SNORM_RELATIVE;
    }
#endif
  }

  if (it)
    system->_last_nl_rnorm = snes->rtol;


/*
  {
    Vec w = snes->vec_sol;

  // cls is a PetscVector wrapper around the Vec in current_local_solution
    PetscVector<Number> cls(static_cast<PetscVector<Number> *>(system->system().current_local_solution.get())->vec());
  // Create new NumericVectors with the right ghosting
  AutoPtr<NumericVector<Number> > ghosted_y_aptr( cls.zero_clone() );
  AutoPtr<NumericVector<Number> > ghosted_w_aptr( cls.zero_clone() );
  // Create PetscVector wrappers around the Vecs
  PetscVector<Number> ghosted_y( static_cast<PetscVector<Number> *>(ghosted_y_aptr.get())->vec() );
  PetscVector<Number> ghosted_w( static_cast<PetscVector<Number> *>(ghosted_w_aptr.get())->vec() );
  //VecCopy(y, ghosted_y.vec());
  VecCopy(w, ghosted_w.vec());

  //ghosted_y.close();
  ghosted_w.close();

  Real max_pen = 0.0;


  damping = problem.computeDamping(ghosted_w, ghosted_y);
  if(damping < 1.0)
  {
    VecScale(y, damping);
    *changed_y = PETSC_TRUE;
  }

//  problem.getNonlinearSystem().set_solution(ghosted_w);
  const NumericVector<Number> * saved_solution = system->currentSolution();

  system->set_solution(ghosted_w);

  FEProblem & problem = static_cast<FEProblem &>(system->subproblem());
  DisplacedProblem & displaced_problem = *problem.getDisplacedProblem();

  if(problem.getDisplacedProblem())
  {
    problem.getDisplacedProblem()->updateMesh(ghosted_w, *problem.getAuxiliarySystem().sys().solution);

    GeometricSearchData & displaced_geom_search_data = displaced_problem.geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = &displaced_geom_search_data._penetration_locators;

    for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
    {
      PenetrationLocator & pen_loc = *it->second;
      if(pen_loc._master_boundary == 10)
      {
        std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        unsigned int slave_boundary = pen_loc._slave_boundary;

        for(unsigned int i=0; i<slave_nodes.size(); i++)
        {
          unsigned int slave_node_num = slave_nodes[i];

          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];
            if(pen_loc.penetrationDistance(slave_node_num) > 0)
            {
//              std::cerr<<"Distance: "<<pen_loc.penetrationDistance(slave_node_num)<<std::endl;
              max_pen = std::max(max_pen, pen_loc.penetrationDistance(slave_node_num));
            }
          }
        }
      }
    }
  }

  libMesh::Parallel::max(max_pen);

//  problem.computePostprocessors(EXEC_RESIDUAL);

//  Real max_pen = problem.getPostprocessorValue("max_penetration");
//  std::cout<<std::endl<<"Max Pen: "<<max_pen<<std::endl<<std::endl;

  if(max_pen > 1e-8)
  {
    std::cout<<std::endl<<"Overpenetration of "<<max_pen<<"  Failing step..."<<std::endl<<std::endl;
#if PETSC_VERSION_LESS_THAN(3,2,0)
    *reason = SNES_DIVERGED_LS_FAILURE;
#else
    *reason = SNES_DIVERGED_LINE_SEARCH;
#endif
  }

  system->set_solution(*saved_solution);


  }

//  if(it)
//    *reason = SNES_DIVERGED_LS_FAILURE;
*/

#if PETSC_VERSION_LESS_THAN(3,3,0)
  // PETSc 3.2.x
  if(*reason == SNES_CONVERGED_PNORM_RELATIVE || *reason == SNES_CONVERGED_FNORM_RELATIVE || *reason == SNES_CONVERGED_FNORM_ABS)
    system->_current_nl_its = it;
#else
  // PETSc 3.3.0+
  if(*reason == SNES_CONVERGED_SNORM_RELATIVE || *reason == SNES_CONVERGED_FNORM_RELATIVE || *reason == SNES_CONVERGED_FNORM_ABS)
    system->_current_nl_its = it;
#endif
  return(0);
}



#if PETSC_VERSION_LESS_THAN(3,3,0)
// PETSc 3.2.x-
PetscErrorCode dampedCheck(SNES /*snes*/, Vec /*x*/, Vec y, Vec w, void *lsctx, PetscBool * changed_y, PetscBool * /*changed_w*/)
#else
// PETSc 3.3.0+
PetscErrorCode dampedCheck(SNESLineSearch /* linesearch */, Vec /*x*/, Vec w, Vec y, PetscBool * /*changed_w*/, PetscBool * changed_y, void *lsctx)
#endif
{
//   From SNESLineSearchSetPostCheck docs:
// +  x - old solution vector
// .  y - search direction vector
// .  w - new solution vector
// .  changed_y - indicates that the line search changed y
// .  changed_w - indicates that the line search changed w
  //w = updated solution = x- scaling*y
  //x = current solution
  //y = updates.

  int ierr = 0;
  Real damping = 1.0;

  FEProblem & problem = *static_cast<FEProblem *>(lsctx);

  TransientNonlinearImplicitSystem & system = problem.getNonlinearSystem().sys();

  // The whole deal here is that we need ghosted versions of vectors y and w (they are parallel, but not ghosted).
  // So to do that I'm going to duplicate current_local_solution (which has the ghosting we want).
  // Then stuff values into the duplicates
  // Then "close()" the vectors which updates their ghosted vaulues.

  {

  // cls is a PetscVector wrapper around the Vec in current_local_solution
  PetscVector<Number> cls(static_cast<PetscVector<Number> *>(system.current_local_solution.get())->vec());
  // Create new NumericVectors with the right ghosting
  AutoPtr<NumericVector<Number> > ghosted_y_aptr( cls.zero_clone() );
  AutoPtr<NumericVector<Number> > ghosted_w_aptr( cls.zero_clone() );
  // Create PetscVector wrappers around the Vecs
  PetscVector<Number> ghosted_y( static_cast<PetscVector<Number> *>(ghosted_y_aptr.get())->vec() );
  PetscVector<Number> ghosted_w( static_cast<PetscVector<Number> *>(ghosted_w_aptr.get())->vec() );

  VecCopy(y, ghosted_y.vec());
  VecCopy(w, ghosted_w.vec());

  ghosted_y.close();
  ghosted_w.close();

  damping = problem.computeDamping(ghosted_w, ghosted_y);
  if(damping < 1.0)
  {
    VecScale(y, damping);
    *changed_y = PETSC_TRUE;
  }
  }

/*
  Real max_pen = 0.0;

  problem.getNonlinearSystem().set_solution(ghosted_w);

  if(problem.getDisplacedProblem())
  {
    problem.getDisplacedProblem()->updateMesh(ghosted_w, *problem.getAuxiliarySystem().sys().solution);

    GeometricSearchData & displaced_geom_search_data = displaced_problem.geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = &displaced_geom_search_data._penetration_locators;

    for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
    {
      PenetrationLocator & pen_loc = *it->second;

      if(pen_loc._master_boundary == 10)
      {


        std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        unsigned int slave_boundary = pen_loc._slave_boundary;

        for(unsigned int i=0; i<slave_nodes.size(); i++)
        {
          unsigned int slave_node_num = slave_nodes[i];

          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];
            if(pen_loc.penetrationDistance(slave_node_num) > 0)
            {
              max_pen = std::max(max_pen, pen_loc.penetrationDistance(slave_node_num));
            }
          }
        }
      }
    }
  }

  libMesh::Parallel::max(max_pen);

//  problem.computePostprocessors(EXEC_RESIDUAL);

//  Real max_pen = problem.getPostprocessorValue("max_penetration");
//  std::cout<<std::endl<<"Max Pen: "<<max_pen<<std::endl<<std::endl;

  if(max_pen > 1e-8)
  {
    std::cerr<<"Damping because max_pen "<<max_pen<<std::endl;
    VecWAXPY(w, -0.098, y, x);
  }
  }
  */
  /*
  {


  PetscVector<Number> cls(static_cast<PetscVector<Number> *>(system.current_local_solution.get())->vec());
  // Create new NumericVectors with the right ghosting
  AutoPtr<NumericVector<Number> > ghosted_y_aptr( cls.zero_clone() );
  AutoPtr<NumericVector<Number> > ghosted_w_aptr( cls.zero_clone() );
  // Create PetscVector wrappers around the Vecs
  PetscVector<Number> ghosted_y( static_cast<PetscVector<Number> *>(ghosted_y_aptr.get())->vec() );
  PetscVector<Number> ghosted_w( static_cast<PetscVector<Number> *>(ghosted_w_aptr.get())->vec() );

  VecCopy(y, ghosted_y.vec());
  VecCopy(w, ghosted_w.vec());

  ghosted_y.close();
  ghosted_w.close();

//   damping = problem.computeDamping(ghosted_w, ghosted_y);
//   if(damping < 1.0)
//   {
//     VecScale(y, damping);
//     *changed_y = PETSC_TRUE;
//   }

  problem.getNonlinearSystem().set_solution(ghosted_w);

  if(problem.getDisplacedProblem())
    problem.getDisplacedProblem()->updateMesh(ghosted_w, *problem.getAuxiliarySystem().sys().solution);


  for(unsigned int i=0; i<libMesh::n_threads(); i++)
    problem.getNonlinearSystem()._constraints[i].jacobianSetup();

  PetscVector<Number> w_pvec(w);

  problem.getNonlinearSystem().setConstraintSlaveValues(w_pvec, false);

  if(problem.getDisplacedProblem())
    problem.getNonlinearSystem().setConstraintSlaveValues(w_pvec, true);
  }
  *
  *changed_w = PETSC_TRUE;

  */

  return(ierr);
}

#if 0
/**
 * Called at the beginning of every nonlinear step (before Jacobian is formed)
 */
PetscErrorCode petscNewtonUpdate(SNES snes, PetscInt /*step*/)
{
  void *ctx = NULL;
  SNESGetApplicationContext(snes, &ctx);

  if (ctx != NULL)
  {
    Executioner *exec = (Executioner *) ctx;    // C strikes again

    exec->updateNewtonStep();
    exec->onNewtonUpdate();
  }

  return 0;
}
#endif

void petscSetupDampers(NonlinearImplicitSystem& sys)
{
  FEProblem * problem = static_cast<FEProblem *>(sys.get_equation_systems().parameters.get<Problem *>("_problem"));
  NonlinearSystem & nl = problem->getNonlinearSystem();
  PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();

#if PETSC_VERSION_LESS_THAN(3,3,0)
  // PETSc 3.2.x-
  SNESLineSearchSetPostCheck(snes, dampedCheck, problem);
#else
  // PETSc 3.3.0+
  SNESLineSearch linesearch;
  SNESGetSNESLineSearch(snes, &linesearch);
  SNESLineSearchSetPostCheck(linesearch, dampedCheck, problem);
#endif
}

void petscSetDefaults(FEProblem & problem)
{
  // dig out Petsc solver
  NonlinearSystem & nl = problem.getNonlinearSystem();
  PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  KSP ksp;
  SNESGetKSP(snes, &ksp);
#if PETSC_VERSION_LESS_THAN(3,2,0)
  // PETSc 3.1.x-
  KSPSetPreconditionerSide(ksp, PC_RIGHT);
#else
  // PETSc 3.2.x+
  KSPSetPCSide(ksp, PC_RIGHT);
#endif
  SNESSetMaxLinearSolveFailures(snes, 1000000);

//  if (problem.hasDampers())
//    SNESLineSearchSetPostCheck(snes, dampedCheck, &problem);

#if PETSC_VERSION_LESS_THAN(3,0,0)
  // PETSc 2.3.3-
  KSPSetConvergenceTest(ksp, petscConverged, &nl);
  SNESSetConvergenceTest(snes, petscNonlinearConverged, &nl);
#else
  // PETSc 3.0.0+

  // In 3.0.0, the context pointer must actually be used, and the
  // final argument to KSPSetConvergenceTest() is a pointer to a
  // routine for destroying said private data context.  In this case,
  // we use the default context provided by PETSc in addition to
  // a few other tests.
  {
    PetscErrorCode ierr = KSPSetConvergenceTest(ksp,
                                                petscConverged,
                                                &nl,
                                                PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = SNESSetConvergenceTest(snes,
                                  petscNonlinearConverged,
                                  &nl,
                                  PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

#endif

//      SNESSetUpdate(snes, petscNewtonUpdate);
//      SNESSetApplicationContext(snes, (void *) executioner);
}
} // Namespace PetscSupport
} // Namespace MOOSE

#endif //LIBMESH_HAVE_PETSC
