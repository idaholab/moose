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
#include "MProblem.h"
#include "NonlinearSystem.h"

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
#include <private/kspimpl.h>
#include <private/snesimpl.h>

namespace Moose 
{
  namespace PetscSupport
  {
    /** The following functionality is encapsulated in the Moose ExecutionBlock but
     * still needed by Pronghorn for the time being
     */
    void petscParseOptions(GetPot & input_file)
    {
      // Set PETSC options:
      int num_petsc_opt = input_file.vector_variable_size("Execution/petsc_options");
      for(int i=0;i<num_petsc_opt;i++) 
      {
        std::string petsc_opts = input_file("Execution/petsc_options","", i);
        PetscOptionsSetValue(petsc_opts.c_str(),PETSC_NULL);
      } // end of i-LOOP...
    
      int num_petsc_opt0 = input_file.vector_variable_size("Execution/petsc_options_iname");
      int num_petsc_opt1 = input_file.vector_variable_size("Execution/petsc_options_value");
      if(num_petsc_opt0 != num_petsc_opt1) 
      {
        printf("Error in reading petsc_options_xxxxx\n");
        libmesh_error();
      }
      for(int i=0;i<num_petsc_opt0;i++) 
      {
        std::string   petsc_opts_iname = input_file("Execution/petsc_options_iname", "", i);
        std::string   petsc_opts_value = input_file("Execution/petsc_options_value", "", i);
        PetscOptionsSetValue(petsc_opts_iname.c_str(),petsc_opts_value.c_str());
      } // end of i-LOOP...
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
      return 0;
    }

    PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void * dummy)
    {
      NonlinearSystem *system = (NonlinearSystem *) dummy;      // C strikes

      // unused
      // TransientNonlinearImplicitSystem * system = dynamic_cast<TransientNonlinearImplicitSystem *>(&_equation_system->get_system("NonlinearSystem"));
  
      //  for(unsigned int var = 0; var < system->n_vars(); var++)
      //    std::cout<<var<<": "<<system->calculate_norm(*system->rhs,var,DISCRETE_L2)<<std::endl;

      *reason = SNES_CONVERGED_ITERATING;

      if (!it)
      {
        /* set parameter for default relative tolerance convergence test */
        snes->ttol = fnorm*snes->rtol;
        system->_initial_residual = fnorm;
      }
      if (fnorm != fnorm)
      {
        PetscInfo(snes,"Failed to converged, function norm is NaN\n");
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
      else if(fnorm >= system->_initial_residual * (1.0/snes->rtol))
      {
        PetscInfo2(snes,"Nonlinear solve was blowing up!",snes->nfuncs,snes->max_funcs);
        *reason = SNES_DIVERGED_LS_FAILURE;
      } 

      if (it && !*reason)
      {
        if (fnorm <= snes->ttol)
        {
          PetscInfo2(snes,"Converged due to function norm %G < %G (relative tolerance)\n",fnorm,snes->ttol);
          *reason = SNES_CONVERGED_FNORM_RELATIVE;
        }
        else if (pnorm < snes->xtol*xnorm)
        {
          PetscInfo3(snes,"Converged due to small update length: %G < %G * %G\n",pnorm,snes->xtol,xnorm);
          *reason = SNES_CONVERGED_PNORM_RELATIVE;
        }
      }
      return(0);
    }


    PetscErrorCode dampedCheck(SNES /*snes*/, Vec /*x*/, Vec y, Vec w, void *lsctx, PetscTruth * changed_y, PetscTruth * /*changed_w*/)
    {
      //w = updated solution = x+ scaling*y
      //x = current solution
      //y = updates.

      int ierr = 0;
      Real damping = 1.0;

      MProblem & problem = *static_cast<MProblem *>(lsctx);
      TransientNonlinearImplicitSystem & system = problem.getNonlinearSystem().sys();

      // The whole deal here is that we need ghosted versions of these vectors.
      // So to do that I'm going to duplicate current_local_solution (which has the ghosting we want).
      // Then stuff values into the duplicates
      // Then "close()" the vectors which updates their ghosted vaulues.

      PetscVector<Number>  ghosted_y(y);
      PetscVector<Number>  ghosted_w(w);

      damping = problem.computeDamping(ghosted_w, ghosted_y);

      if(damping < 1.0)
      {
        VecScale(y, damping);
        *changed_y = PETSC_TRUE;
      }

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

    void petscSetDefaults(MProblem & problem)
    {
      // dig out Petsc solver
      NonlinearSystem & nl = problem.getNonlinearSystem();
      PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
      SNES snes = petsc_solver->snes();
      KSP ksp;
      SNESGetKSP(snes, &ksp);
      KSPSetPreconditionerSide(ksp, PC_RIGHT);
      SNESSetMaxLinearSolveFailures(snes, 1000000);

      if (problem.hasDampers())
        SNESLineSearchSetPostCheck(snes, dampedCheck, &problem);

#if PETSC_VERSION_LESS_THAN(3,0,0)
      KSPSetConvergenceTest(ksp, petscConverged, &nl);
      SNESSetConvergenceTest(snes, petscNonlinearConverged, &nl);
#else

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
      }
    
#endif
    
//      SNESSetUpdate(snes, petscNewtonUpdate);
//      SNESSetApplicationContext(snes, (void *) executioner);
    }
  }
}


#endif //LIBMESH_HAVE_PETSC
