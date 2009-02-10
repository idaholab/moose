#include "PhysicsBasedPreconditioner.h"

//local includes
#include "Moose.h"

//libMesh Includes
#include "libmesh_common.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "linear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

void
PhysicsBasedPreconditioner::init ()
{
  Moose::perf_log.push("init()","PhysicsBasedPreconditioner");

  if(!_equation_systems)
  {
    std::cerr<<"EquationSystems must be set on PhysicsBasedPreconditioner before use!"<<std::endl;
    libmesh_error();
  }

  if(!_compute_jacobian_block)
  {
    std::cerr<<"ComputeJacobianBlock must be set on PhysicsBasedPreconditioner before use!"<<std::endl;
    libmesh_error();
  }

  TransientNonlinearImplicitSystem & system = _equation_systems->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  if(_preconditioners.size() == 0)
    _preconditioners.resize(_equation_systems->n_systems());
    
  //Start at 1 because "NonlinearSystem" is zero
  for(unsigned int sys=1; sys<_equation_systems->n_systems(); sys++)
  {
    //By convention
    unsigned int system_var = sys-1;
      
    if(!_preconditioners[system_var])
      _preconditioners[system_var] = Preconditioner<Number>::build();

    Preconditioner<Number> * preconditioner = _preconditioners[system_var];

    LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);

    preconditioner->set_matrix(*u_system.matrix);

    preconditioner->set_type(AMG_PRECOND);

    _compute_jacobian_block(*system.current_local_solution,*u_system.matrix,u_system,system_var,system_var);

    preconditioner->init();
  }

  Moose::perf_log.pop("init()","PhysicsBasedPreconditioner");
}

void
PhysicsBasedPreconditioner::apply(const NumericVector<Number> & x, NumericVector<Number> & y)
{
    Moose::perf_log.push("apply()","PhysicsBasedPreconditioner");

    TransientNonlinearImplicitSystem & system = _equation_systems->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
    
    //Start at 1 because "NonlinearSystem" is zero
    for(unsigned int sys=1; sys<_equation_systems->n_systems(); sys++)
    {
      //By convention
      unsigned int system_var = sys-1;
      
      LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);

      MeshBase & mesh = _equation_systems->get_mesh();

      //Copy rhs from the big system into the small one
      copyVarValues(mesh,0,system_var,x,sys,0,*u_system.rhs);

      //Apply the preconditioner to the small system
      _preconditioners[system_var]->apply(*u_system.rhs,*u_system.solution);

      //Copy solution from small system into the big one
      copyVarValues(mesh,sys,0,*u_system.solution,0,system_var,y);
    }

    Moose::perf_log.pop("apply()","PhysicsBasedPreconditioner");
}

void
PhysicsBasedPreconditioner::copyVarValues(MeshBase & mesh,
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
