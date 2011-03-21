#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

#include "Moose.h"
#include "Executioner.h"

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
#if 0
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
      MooseSystem *moose_system = (MooseSystem *) dummy;      // C strikes

      *reason = KSP_CONVERGED_ITERATING;

      //If it's the beginning of a new set of iterations, reset last_rnorm
      if(!n)
        moose_system->_last_rnorm = 1e99;

      PetscReal norm_diff = std::fabs(rnorm - moose_system->_last_rnorm);
  
      if(norm_diff < moose_system->_l_abs_step_tol)
      {
        *reason = KSP_CONVERGED_RTOL;
        return(0);
      }

      moose_system->_last_rnorm = rnorm;

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
      MooseSystem *moose_system = (MooseSystem *) dummy;      // C strikes

      // unused
      // TransientNonlinearImplicitSystem * system = dynamic_cast<TransientNonlinearImplicitSystem *>(&_equation_system->get_system("NonlinearSystem"));
  
      //  for(unsigned int var = 0; var < system->n_vars(); var++)
      //    std::cout<<var<<": "<<system->calculate_norm(*system->rhs,var,DISCRETE_L2)<<std::endl;

      *reason = SNES_CONVERGED_ITERATING;

      if (!it)
      {
        /* set parameter for default relative tolerance convergence test */
        snes->ttol = fnorm*snes->rtol;
        moose_system->_initial_residual = fnorm;
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
      else if(fnorm >= moose_system->_initial_residual * (1.0/snes->rtol))
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

    void petscSetDefaults(MooseSystem &moose_system, Executioner *executioner)
    {
      PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(moose_system.getNonlinearSystem()->nonlinear_solver.get());
      SNES snes = petsc_solver->snes();
      KSP ksp;
      SNESGetKSP(snes, &ksp);
      KSPSetPreconditionerSide(ksp, PC_RIGHT);
      SNESSetMaxLinearSolveFailures(snes, 1000000);

      if(moose_system.hasDampers())
        SNESLineSearchSetPostCheck(snes, dampedCheck, moose_system.getEquationSystems());

#if PETSC_VERSION_LESS_THAN(3,0,0)
      KSPSetConvergenceTest(ksp, petscConverged, &moose_system);
      SNESSetConvergenceTest(snes, petscNonlinearConverged, &moose_system);
#else

      // In 3.0.0, the context pointer must actually be used, and the
      // final argument to KSPSetConvergenceTest() is a pointer to a
      // routine for destroying said private data context.  In this case,
      // we use the default context provided by PETSc in addition to
      // a few other tests.
      {
        PetscErrorCode ierr = KSPSetConvergenceTest(ksp,
                                                    petscConverged,
                                                    &moose_system,
                                                    PETSC_NULL);
        CHKERRABORT(libMesh::COMM_WORLD,ierr);
      }
    
#endif
    
      SNESSetUpdate(snes, petscNewtonUpdate);
      SNESSetApplicationContext(snes, (void *) executioner);
    }

    PetscErrorCode dampedCheck(SNES /*snes*/, Vec /*x*/, Vec y, Vec w, void *lsctx, PetscTruth * changed_y, PetscTruth * /*changed_w*/)
    {
      //w = updated solution = x+ scaling*y
      //x = current solution
      //y = updates.

      int ierr = 0;
      Real damping = 1.0;

      EquationSystems * equation_systems = static_cast<EquationSystems *>(lsctx);

      MooseSystem * moose_system = equation_systems->parameters.get<MooseSystem *>("moose_system");

      TransientNonlinearImplicitSystem& system =
        equation_systems->get_system<TransientNonlinearImplicitSystem> ("NonlinearSystem");

      // The whole deal here is that we need ghosted versions of these vectors.
      // So to do that I'm going to duplicate current_local_solution (which has the ghosting we want).
      // Then stuff values into the duplicates
      // Then "close()" the vectors which updates their ghosted vaulues.
      
      Vec current_local_solution = static_cast<PetscVector<Number> *>(system.current_local_solution.get())->vec();    

      Vec ghosted_w;
      Vec ghosted_y;

      VecDuplicate(current_local_solution, &ghosted_w);
      VecDuplicate(current_local_solution, &ghosted_y);

      VecCopy(w, ghosted_w);
      VecCopy(y, ghosted_y);

      PetscVector<Number>  ghosted_update_vec_y(ghosted_y);
      PetscVector<Number>  ghosted_update_vec_w(ghosted_w);

      ghosted_update_vec_y.close();
      ghosted_update_vec_w.close();

      damping = moose_system->computeDamping(ghosted_update_vec_w, ghosted_update_vec_y);

      if(damping < 1.0)
      {
        std::cout<<"Damping Factor: "<<damping<<std::endl;
        VecScale(y, damping);
        *changed_y = PETSC_TRUE;
      }

      VecDestroy(ghosted_w);
      VecDestroy(ghosted_y);

      return(ierr);
    }
#endif
  }
}


#endif //LIBMESH_HAVE_PETSC
