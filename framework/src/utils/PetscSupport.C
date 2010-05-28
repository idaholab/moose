#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

#include "Moose.h"

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
    PetscReal l_abs_step_tol = -1;
    EquationSystems *_equation_system = NULL;

    void (*_compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar) = NULL;
    void (*_compute_residual) (const NumericVector<Number>& soln, NumericVector<Number>& residual) = NULL;

    std::vector<PetscPreconditioner<Number> *> preconditioners;

    /** The following functionality is encapsulated in the Moose ExecutionBlock but
     * still needed by Pronghorn for the time being
     */
    void petscParseOptions(GetPot & input_file)
    {
      l_abs_step_tol = input_file("Execution/l_abs_step_tol",-1.0);

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
      *reason = KSP_CONVERGED_ITERATING;

      //See if the solver has stagnated
      static PetscReal last_rnorm = 0;

      //If it's the beginning of a new set of iterations, reset last_rnorm
      if(!n)
        last_rnorm = 1e99;

      PetscReal norm_diff = std::fabs(rnorm-last_rnorm);
  
      if(norm_diff < l_abs_step_tol)
      {
        *reason = KSP_CONVERGED_RTOL;
        return(0);
      }

      last_rnorm = rnorm;

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

    PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void * /*dummy*/)
    {
      // unused
      // TransientNonlinearImplicitSystem * system = dynamic_cast<TransientNonlinearImplicitSystem *>(&_equation_system->get_system("NonlinearSystem"));
  
      //  for(unsigned int var = 0; var < system->n_vars(); var++)
      //    std::cout<<var<<": "<<system->calculate_norm(*system->rhs,var,DISCRETE_L2)<<std::endl;

      static Real initial_residual = 0;

      *reason = SNES_CONVERGED_ITERATING;

      if (!it)
      {
        /* set parameter for default relative tolerance convergence test */
        snes->ttol = fnorm*snes->rtol;
        initial_residual = fnorm;
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
      else if(fnorm >= initial_residual * (1.0/snes->rtol))
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

    PetscErrorCode MatrixFreePreconditionerSetup(void * /*ctx*/)
    {
      Moose::perf_log.push("MatrixFreePreconditionerSetup()","Preconditioning");

      TransientNonlinearImplicitSystem & system = _equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

      if(preconditioners.size() == 0)
        preconditioners.resize(_equation_system->n_systems());
    
      //Start at 1 because "NonlinearSystem" is zero
      for(unsigned int sys=1; sys<_equation_system->n_systems(); sys++)
      {
        //By convention
        unsigned int system_var = sys-1;
      
        if(!preconditioners[system_var])
          preconditioners[system_var] = new PetscPreconditioner<Number>;

        PetscPreconditioner<Number> * preconditioner = preconditioners[system_var];

        LinearImplicitSystem & u_system = _equation_system->get_system<LinearImplicitSystem>(sys);

        preconditioner->set_matrix(*u_system.matrix);

        preconditioner->set_type(AMG_PRECOND);

        _compute_jacobian_block(*system.current_local_solution,*u_system.matrix,u_system,system_var,system_var);

        preconditioner->init();
      }

      Moose::perf_log.pop("MatrixFreePreconditionerSetup()","Preconditioning");

      return 0;
    }

    void copyVarValues(MeshBase & mesh,
                       const unsigned int from_system, const unsigned int from_var, const NumericVector<Number> & from_vector,
                       const unsigned int to_system, const unsigned int to_var, NumericVector<Number> & to_vector)
    {
      MeshBase::node_iterator it = mesh.local_nodes_begin();
      MeshBase::node_iterator it_end = mesh.local_nodes_end();

      for(;it!=it_end;++it)
      {
        Node * node = *it;

        //The zeroes are for the component.
        //If we ever want to use non-lagrange elements we'll have to change that.
        unsigned int from_dof = node->dof_number(from_system,from_var,0);
        unsigned int to_dof = node->dof_number(to_system,to_var,0);

        to_vector.set(to_dof,from_vector(from_dof));
      }
    }  

    PetscErrorCode MatrixFreePreconditioner(void * /*ctx*/,Vec x, Vec y)
    {
      Moose::perf_log.push("MatrixFreePreconditioner()","Preconditioning");

      PetscVector<Number> x_vec(x);
      PetscVector<Number> y_vec(y);

      // unused...
      // TransientNonlinearImplicitSystem & system = _equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
    
      //Start at 1 because "NonlinearSystem" is zero
      for(unsigned int sys=1; sys<_equation_system->n_systems(); sys++)
      {
        //By convention
        unsigned int system_var = sys-1;
      
        LinearImplicitSystem & u_system = _equation_system->get_system<LinearImplicitSystem>(sys);

        PetscLinearSolver<Real> * u_system_solver = static_cast<PetscLinearSolver<Real>*>(u_system.linear_solver.get());

        KSP u_system_ksp = u_system_solver->ksp();
        PC pc;
        KSPGetPC(u_system_ksp,&pc);

        MeshBase & mesh = _equation_system->get_mesh();

        copyVarValues(mesh,0,system_var,x_vec,sys,0,*u_system.rhs);

        preconditioners[system_var]->apply(*u_system.rhs,*u_system.solution);

        copyVarValues(mesh,sys,0,*u_system.solution,0,system_var,y_vec);      
      }

      Moose::perf_log.pop("MatrixFreePreconditioner()","Preconditioning");
    
      return 0;
    }

    void petscSetDefaults(EquationSystems * es, TransientNonlinearImplicitSystem & system, void (*compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar), void (*compute_residual) (const NumericVector<Number>& soln, NumericVector<Number>& residual))
    {
      _equation_system = es;
      _compute_jacobian_block = compute_jacobian_block;
      _compute_residual = compute_residual;
    
      PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(system.nonlinear_solver.get());
      SNES snes = petsc_solver->snes();
      KSP ksp;
      SNESGetKSP(snes, &ksp);
      KSPSetPreconditionerSide(ksp, PC_RIGHT);
      SNESSetMaxLinearSolveFailures(snes, 1000000);

#if PETSC_VERSION_LESS_THAN(3,0,0)
      KSPSetConvergenceTest(ksp, petscConverged, PETSC_NULL);
      SNESSetConvergenceTest(snes, petscNonlinearConverged, NULL);
#else

      // In 3.0.0, the context pointer must actually be used, and the
      // final argument to KSPSetConvergenceTest() is a pointer to a
      // routine for destroying said private data context.  In this case,
      // we use the default context provided by PETSc in addition to
      // a few other tests.
      {
        void *ctx;
        PetscErrorCode ierr = KSPDefaultConvergedCreate(&ctx);
        CHKERRABORT(libMesh::COMM_WORLD,ierr);
    
        ierr = KSPSetConvergenceTest(ksp,
                                     petscConverged,
                                     ctx,
                                     KSPDefaultConvergedDestroy);
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
      Real facmin = 1.0;
      Real max_fraction = 0.5;
    
      *flag = PETSC_TRUE;

      EquationSystems * equation_systems = static_cast<EquationSystems *>(lsctx);
    
      TransientNonlinearImplicitSystem& system =
        equation_systems->get_system<TransientNonlinearImplicitSystem> ("NonlinearSystem");
    
      //create PetscVector
      PetscVector<Number>  update_vec_x(x);
      PetscVector<Number>  update_vec_y(y);
      PetscVector<Number>  update_vec_w(w);

      //vector  stores updated solution
      update_vec_w.zero();
      update_vec_w.add(1.,update_vec_x);
      update_vec_w.add(-1,update_vec_y);

      if(!equation_systems->parameters.have_parameter<std::vector<std::string> >("pbd_var") )
        facmin = 1.0;
      else
      {
        int n_pbd_var = equation_systems->parameters.get<std::vector<std::string> >("pbd_var").size();
        std::set<unsigned int> var_indices;

        max_fraction = equation_systems->parameters.get<Real>("pbd_max_fraction");

        std::cout << "max_fraction: " << max_fraction << std::endl;
      
        for(int j=0;j<n_pbd_var;j++)
        {
          std::string var_name = equation_systems->parameters.get<std::vector<std::string> >("pbd_var")[j].c_str();
          int i = system.variable_number(var_name.c_str());
          
          system.local_dof_indices(i, var_indices);
          std::set<unsigned int>:: iterator it = var_indices.begin();
          const std::set<unsigned int>::iterator it_end = var_indices.end();
          for(; it !=it_end; ++it)
          {
            if( update_vec_w(*it) < max_fraction*update_vec_x(*it) )
            {
              Real fac = max_fraction*update_vec_x(*it)/update_vec_y(*it);
              if( fac < facmin )
                facmin = fac;
            }
            else if( update_vec_w(*it) > (1+max_fraction)*update_vec_x(*it) )
            {
              Real fac = -max_fraction*update_vec_x(*it)/update_vec_y(*it);
              if( fac < facmin )
                facmin = fac;
            }
          }
        }
      }

      Parallel::min(facmin);
    
      facmin = -facmin;
    
      std::cout << std::endl;
      std::cout << "facmin..: " << facmin << std::endl;
      std::cout << std::endl;
    
      //this is standard newton update with damping parameter
      ierr = VecNorm(y,NORM_2,ynorm);
      ierr = VecWAXPY(w,facmin,y,x);
      ierr = SNESComputeFunction(snes,w,g);
      ierr = VecNorm(g,NORM_2,gnorm);
      return(ierr);
    

    } 
  }
}


#endif //LIBMESH_HAVE_PETSC
