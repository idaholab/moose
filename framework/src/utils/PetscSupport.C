#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

#include "Moose.h"
#include "MooseSystem.h"

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

    void petscSetDefaults(MooseSystem &moose_system)
    {
      PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(moose_system.getNonlinearSystem()->nonlinear_solver.get());
      SNES snes = petsc_solver->snes();
      KSP ksp;
      SNESGetKSP(snes, &ksp);
      KSPSetPreconditionerSide(ksp, PC_RIGHT);
      SNESSetMaxLinearSolveFailures(snes, 1000000);

//      SNESLineSearchSet(snes, petscPhysicsBasedLineSearch, moose_system.getEquationSystems());

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
    
      /*
        PC pc;
        KSPGetPC(ksp,&pc);
        PCSetType(pc,PCSHELL);
        PCShellSetSetUp(pc,MatrixFreePreconditionerSetup);
        PCShellSetApply(pc,MatrixFreePreconditioner);
      */
    }
    
    PetscErrorCode petscPhysicsBasedLineSearch(SNES snes,void *lsctx,Vec x,Vec /*f*/,Vec g,Vec y,Vec w, PetscReal /*fnorm*/,PetscReal *ynorm,PetscReal *gnorm,PetscTruth *flag)
    {
      //w = updated solution = x+ scaling*y
      //x = current solution
      //y = updates.
      // for simple newton use w = x-y


      int ierr;
      Real facmin = 0.2;
      Real max_fraction = 0.5;

      *flag = PETSC_TRUE;

      EquationSystems * equation_systems = static_cast<EquationSystems *>(lsctx);

      MeshBase & mesh = equation_systems->get_mesh();
    
      TransientNonlinearImplicitSystem& system =
        equation_systems->get_system<TransientNonlinearImplicitSystem> ("NonlinearSystem");

      unsigned int sys = system.number();
    
      //create PetscVector
      PetscVector<Number>  update_vec_x(x);
      PetscVector<Number>  update_vec_y(y);
      PetscVector<Number>  update_vec_w(w);

      //vector  stores updated solution
      update_vec_w.zero();
      update_vec_w.add(1.,update_vec_x);
      update_vec_w.add(-1.0,update_vec_y);
/*
      MeshBase::const_node_iterator nd     = mesh.local_nodes_begin();
      MeshBase::const_node_iterator nd_end = mesh.local_nodes_end();
      MeshBase::const_node_iterator nd_it  = mesh.local_nodes_begin();;

      for(nd_it = nd;nd_it != nd_end; ++nd_it)
      {
        Node * node = *nd_it;

        unsigned int dof = node->dof_number(sys, 0, 0);
        
//        std::cout<<"x: "<<update_vec_x(dof)<<std::endl;

//        if(dof == 84)
//          std::cout<<"dof 84!"<<std::endl<<"x: "<<update_vec_x(dof)<<std::endl<<"y: "<<update_vec_y(dof)<<std::endl;          

        if( update_vec_w(dof) <= 0 )
        {
          if(update_vec_y(dof))
          {    
            Real fac = (update_vec_x(dof) - .000045438)/update_vec_y(dof);
            if( fac < facmin )
              facmin = fac;
          }
        }
        else if( update_vec_w(dof) >= 1 )
        {
          if(update_vec_y(dof))
          {    
            Real fac = (update_vec_x(dof) - .99995457)/update_vec_y(dof);
            if( fac < facmin )
              facmin = fac;
          }
        }
      }

      Parallel::min(facmin);

      if(facmin < 1.0)
      {
        std::cout << std::endl;
        std::cout << "facmin..: " << facmin << std::endl;
        std::cout << std::endl;
      }
*/  
      update_vec_w.zero();
      //this is standard newton update with damping parameter
      ierr = VecNorm(y,NORM_2,ynorm);
      ierr = VecWAXPY(w,-facmin,y,x);
/*
      for(nd_it = nd;nd_it != nd_end; ++nd_it)
      {
        Node * node = *nd_it;

        unsigned int dof = node->dof_number(sys, 0, 0);

        if(update_vec_w(dof) <= 0 || update_vec_w(dof) >= 1)
          std::cout<<"w: "<<dof<<" = "<<update_vec_w(dof)<<std::endl;
      }
*/
      ierr = SNESComputeFunction(snes,w,g);
      ierr = VecNorm(g,NORM_2,gnorm);
      return(ierr);
    

    } 
  }
}


#endif //LIBMESH_HAVE_PETSC
