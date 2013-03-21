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

#include "PhysicsBasedPreconditioner.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "MooseEnum.h"

//libMesh Includes
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<PhysicsBasedPreconditioner>()
{
  InputParameters params = validParams<MoosePreconditioner>();

  params.addRequiredParam<std::vector<std::string> >("solve_order", "The order the block rows will be solved in.  Put the name of variables here to stand for solving that variable's block row.  A variable may appear more than once (to create cylces if you like).");
  params.addRequiredParam<std::vector<std::string> >("preconditioner", "TODO: docstring");

  params.addParam<std::vector<std::string> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<std::string> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");


  return params;
}

PhysicsBasedPreconditioner::PhysicsBasedPreconditioner (const std::string & name, InputParameters params) :
    MoosePreconditioner(name, params),
    Preconditioner<Number>(),
    _nl(_fe_problem.getNonlinearSystem())
{
  unsigned int num_systems = _nl.sys().n_vars();
  _systems.resize(num_systems);
  _preconditioners.resize(num_systems);
  _off_diag.resize(num_systems);
  _off_diag_mats.resize(num_systems);
  _pre_type.resize(num_systems);

  { // Setup the Coupling Matrix so MOOSE knows what we're doing
    NonlinearSystem & nl = _fe_problem.getNonlinearSystem();
    unsigned int n_vars = nl.nVariables();

    CouplingMatrix * cm = new CouplingMatrix(n_vars);

    bool full = false; //getParam<bool>("full"); // TODO: add a FULL option for PBP

    if (!full)
    {
      // put 1s on diagonal
      for (unsigned int i = 0; i < n_vars; i++)
        (*cm)(i, i) = 1;

      // off-diagonal entries
      std::vector<std::vector<unsigned int> > off_diag(n_vars);
      for (unsigned int i = 0; i < getParam<std::vector<std::string> >("off_diag_row").size(); i++)
      {
        unsigned int row = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_row")[i]).index();
        unsigned int column = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_column")[i]).index();
        (*cm)(row, column) = 1;
      }

      // TODO: handle coupling entries between NL-vars and SCALAR-vars
    }
    else
    {
      for (unsigned int i = 0; i < n_vars; i++)
        for (unsigned int j = 0; j < n_vars; j++)
          (*cm)(i,j) = 1;
    }

    _fe_problem.setCouplingMatrix(cm);
  }

  // PC types
  const std::vector<std::string> & pc_types = getParam<std::vector<std::string> >("preconditioner");
  for (unsigned int i = 0; i < num_systems; i++)
    _pre_type[i] = Utility::string_to_enum<PreconditionerType>(pc_types[i]);

  // solve order
  const std::vector<std::string> & solve_order = getParam<std::vector<std::string> >("solve_order");
  _solve_order.resize(solve_order.size());
  for (unsigned int i = 0; i < solve_order.size(); i++)
    _solve_order[i] = _nl.sys().variable_number(solve_order[i]);

  // diag and off-diag systems
  unsigned int n_vars = _nl.sys().n_vars();

  // off-diagonal entries
  const std::vector<std::string> & odr = getParam<std::vector<std::string> >("off_diag_row");
  const std::vector<std::string> & odc = getParam<std::vector<std::string> >("off_diag_column");
  std::vector<std::vector<unsigned int> > off_diag(n_vars);
  for (unsigned int i = 0; i < odr.size(); i++)
  {
    unsigned int row = _nl.sys().variable_number(odr[i]);
    unsigned int column = _nl.sys().variable_number(odc[i]);
    off_diag[row].push_back(column);
  }
  // Add all of the preconditioning systems
  for (unsigned int var = 0; var < n_vars; var++)
    addSystem(var, off_diag[var], _pre_type[var]);

  // We don't want to be computing the big Jacobian!
  _nl.sys().nonlinear_solver->jacobian = NULL;
  _nl.sys().nonlinear_solver->attach_preconditioner(this);

  // If using PETSc, use the right PETSc option
#ifdef LIBMESH_HAVE_PETSC
  std::vector<MooseEnum> petsc_options(1, MooseEnum("-snes_mf", "-snes_mf"));  // SNES Matrix Free
  std::vector<std::string> petsc_inames, petsc_values;

  _fe_problem.storePetscOptions(petsc_options, petsc_inames, petsc_values);
  Moose::PetscSupport::petscSetOptions(_fe_problem);
#endif
}

PhysicsBasedPreconditioner::~PhysicsBasedPreconditioner ()
{
  this->clear();

  std::vector<Preconditioner<Number> *>::iterator it;
  for (it = _preconditioners.begin(); it != _preconditioners.end(); ++it)
    delete *it;
}

void
PhysicsBasedPreconditioner::addSystem(unsigned int var, std::vector<unsigned int> off_diag, PreconditionerType type)
{
  std::string var_name = _nl.sys().variable_name(var);

  LinearImplicitSystem & precond_system = _fe_problem.es().add_system<LinearImplicitSystem>(var_name+"_system");
  precond_system.assemble_before_solve = false;

  const std::set<SubdomainID> * active_subdomains = _nl.getVariableBlocks(var);
  precond_system.add_variable(var_name+"_prec", _nl.sys().variable(var).type(), active_subdomains);

  _systems[var] = &precond_system;
  _pre_type[var] = type;

  _off_diag_mats[var].resize(off_diag.size());
  for(unsigned int i=0;i<off_diag.size();i++)
  {
    //Add the matrix to hold the off-diagonal piece
    _off_diag_mats[var][i] = &precond_system.add_matrix(_nl.sys().variable_name(off_diag[i]));
  }

  _off_diag[var] = off_diag;
}

void
PhysicsBasedPreconditioner::init ()
{
  Moose::perf_log.push("init()","PhysicsBasedPreconditioner");

  // Tell libMesh that this is initialized!
  _is_initialized = true;

  const unsigned int num_systems = _systems.size();

  //If no order was specified, just solve them in increasing order
  if(_solve_order.size() == 0)
  {
    _solve_order.resize(num_systems);
    for(unsigned int i=0;i<num_systems;i++)
      _solve_order[i]=i;
  }

  //Loop over variables
  for(unsigned int system_var=0; system_var<num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    if(!_preconditioners[system_var])
      _preconditioners[system_var] = Preconditioner<Number>::build();

    // we have to explicitly set the matrix in the preconditioner, because h-adaptivity could have changed it and we have to work with the current one
    Preconditioner<Number> * preconditioner = _preconditioners[system_var];
    preconditioner->set_matrix(*u_system.matrix);
    preconditioner->set_type(_pre_type[system_var]);

    preconditioner->init();
  }

  Moose::perf_log.pop("init()","PhysicsBasedPreconditioner");
}

void
PhysicsBasedPreconditioner::setup()
{
  const unsigned int num_systems = _systems.size();

  //Loop over variables
  for(unsigned int system_var=0; system_var<num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    //Compute the diagonal block... storing the result in the system matrix
    _fe_problem.computeJacobianBlock(*u_system.matrix, u_system, system_var, system_var);

    for(unsigned int diag=0;diag<_off_diag[system_var].size();diag++)
    {
      unsigned int coupled_var = _off_diag[system_var][diag];
      std::string coupled_name = _nl.sys().variable_name(coupled_var);
      _fe_problem.computeJacobianBlock(*_off_diag_mats[system_var][diag], u_system, system_var, coupled_var);
    }
  }
}

void
PhysicsBasedPreconditioner::apply(const NumericVector<Number> & x, NumericVector<Number> & y)
{
  Moose::perf_log.push("apply()","PhysicsBasedPreconditioner");

  const unsigned int num_systems = _systems.size();

  MooseMesh & mesh = _fe_problem.mesh();

  //Zero out the solution vectors
  for(unsigned int sys=0; sys<num_systems; sys++)
    _systems[sys]->solution->zero();

  //Loop over solve order
  for(unsigned int i=0; i<_solve_order.size(); i++)
  {
    unsigned int system_var = _solve_order[i];

    LinearImplicitSystem & u_system = *_systems[system_var];

    //Copy rhs from the big system into the small one
    MoosePreconditioner::copyVarValues(mesh,
        _nl.sys().number(),system_var,x,
        u_system.number(),0,*u_system.rhs);

    //Modify the RHS by subtracting off the matvecs of the solutions for the other preconditioning
    //systems with the off diagonal blocks in this system.
    for(unsigned int diag=0;diag<_off_diag[system_var].size();diag++)
    {
      unsigned int coupled_var = _off_diag[system_var][diag];
      LinearImplicitSystem & coupled_system = *_systems[coupled_var];
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
    _preconditioners[system_var]->apply(*u_system.rhs, *u_system.solution);

    //Copy solution from small system into the big one
    //copyVarValues(mesh,sys,0,*u_system.solution,0,system_var,y);
  }

  //Copy the solutions out
  for(unsigned int system_var=0; system_var<num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    MoosePreconditioner::copyVarValues(mesh,
        u_system.number(),0,*u_system.solution,
        _nl.sys().number(),system_var,y);
  }

  y.close();

  Moose::perf_log.pop("apply()","PhysicsBasedPreconditioner");
}

void
PhysicsBasedPreconditioner::clear ()
{
}
