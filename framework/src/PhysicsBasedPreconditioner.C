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
#include "sparse_matrix.h"

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

  // -1 to take into account the Nonlinear system itself
  const unsigned int num_systems = _equation_systems->n_systems()-1;

  if(_preconditioners.size() == 0)
    _preconditioners.resize(num_systems);

  //If no order was specified, just solve them in increasing order
  if(_solve_order.size() == 0)
  {
    _solve_order.resize(num_systems);
    for(unsigned int i=0;i<num_systems;i++)
      _solve_order[i]=i;
  }

  if(_off_diag.size() != num_systems)
    _off_diag.resize(num_systems);

  if(_off_diag_mats.size() != num_systems)
    _off_diag_mats.resize(num_systems);

  //Default to using AMG
  if(_pre_type.size() != num_systems)
  {
    _pre_type.resize(num_systems);
    for(unsigned int i=0;i<num_systems;i++)
      _pre_type[i]=AMG_PRECOND;
  }

  //Loop over variables
  for(unsigned int system_var=0; system_var<num_systems; system_var++)
  {
    //+1 to take into acount the Nonlinear system
    unsigned int sys = system_var+1;
    
    if(!_preconditioners[system_var])
      _preconditioners[system_var] = Preconditioner<Number>::build();

    if(_off_diag_mats[system_var].size() != _off_diag[system_var].size())
      _off_diag_mats[system_var].resize(_off_diag[system_var].size());

    Preconditioner<Number> * preconditioner = _preconditioners[system_var];

    LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);

    preconditioner->set_matrix(*u_system.matrix);

    preconditioner->set_type(_pre_type[system_var]);

    //Compute the diagonal block... storing the result in the system matrix
    _compute_jacobian_block(*system.current_local_solution,*u_system.matrix,u_system,system_var,system_var);

//    std::cout<<_equation_systems->get_system<TransientNonlinearImplicitSystem>(0).variable_name(system_var)<<std::endl;

    for(unsigned int diag=0;diag<_off_diag[system_var].size();diag++)
    {
      unsigned int coupled_var = _off_diag[system_var][diag];

//      std::cout<<" "<<_equation_systems->get_system<TransientNonlinearImplicitSystem>(0).variable_name(coupled_var)<<std::endl;
      
      //System 0 is the Nonlinear system
      std::string coupled_name = _equation_systems->get_system(0).variable_name(coupled_var);
      
      _off_diag_mats[system_var][diag] = &u_system.get_matrix(coupled_name);
        
      _compute_jacobian_block(*system.current_local_solution,*_off_diag_mats[system_var][diag],
                              u_system,system_var,coupled_var);
    }

    preconditioner->init();
  }

  Moose::perf_log.pop("init()","PhysicsBasedPreconditioner");
}

void
PhysicsBasedPreconditioner::apply(const NumericVector<Number> & x, NumericVector<Number> & y)
{
  Moose::perf_log.push("apply()","PhysicsBasedPreconditioner");

  // -1 to take into account the Nonlinear system itself
  const unsigned int num_systems = _equation_systems->n_systems()-1;
  
  TransientNonlinearImplicitSystem & system = _equation_systems->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  MeshBase & mesh = _equation_systems->get_mesh();

  //Zero out the solution vectors
  for(unsigned int sys=1; sys<num_systems+1; sys++)
  {
    LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);
    u_system.solution->zero();
  }
  
  //Loop over solve order
  for(unsigned int i=0; i<_solve_order.size(); i++)
  {
    //+1 to take into acount the Nonlinear system
    unsigned int sys = _solve_order[i]+1;
    
    //By convention
    unsigned int system_var = sys-1;
      
    LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);

    //Copy rhs from the big system into the small one
    copyVarValues(mesh,0,system_var,x,sys,0,*u_system.rhs);

//    std::cout<<_equation_systems->get_system<TransientNonlinearImplicitSystem>(0).variable_name(system_var)<<std::endl;

    //Modify the RHS by subtracting off the matvecs of the solutions for the other preconditioning
    //systems with the off diagonal blocks in this system.
    for(unsigned int diag=0;diag<_off_diag[system_var].size();diag++)
    {
      unsigned int coupled_var = _off_diag[system_var][diag];

//      std::cout<<" "<<_equation_systems->get_system<TransientNonlinearImplicitSystem>(0).variable_name(coupled_var)<<std::endl;

      //By convention
      unsigned int coupled_sys = coupled_var+1;
      
      LinearImplicitSystem & coupled_system = _equation_systems->get_system<LinearImplicitSystem>(coupled_sys);

      SparseMatrix<Number> & off_diag = *_off_diag_mats[system_var][diag];

      NumericVector<Number> & rhs = *u_system.rhs;

      //This next bit computes rhs -= A*coupled_solution
      //It does what it does because there is no vector_mult_sub()
      rhs.close();
      rhs.scale(-1.0);
      rhs.close();
      off_diag.vector_mult_add(rhs,*coupled_system.solution);
      rhs.close();
      rhs.scale(-1.0);
      rhs.close();
    }      

    //Apply the preconditioner to the small system
    _preconditioners[system_var]->apply(*u_system.rhs,*u_system.solution);
    
    //Copy solution from small system into the big one
    //copyVarValues(mesh,sys,0,*u_system.solution,0,system_var,y);
  }

  //Copy the solutions out
  for(unsigned int sys=1; sys<num_systems+1; sys++)
  {
    //By convention
    unsigned int system_var = sys-1;

    LinearImplicitSystem & u_system = _equation_systems->get_system<LinearImplicitSystem>(sys);

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
