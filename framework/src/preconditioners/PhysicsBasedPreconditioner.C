//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsBasedPreconditioner.h"

// MOOSE includes
#include "ComputeJacobianBlocksThread.h"
#include "FEProblem.h"
#include "MooseEnum.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/coupling_matrix.h"

registerMooseObjectAliased("MooseApp", PhysicsBasedPreconditioner, "PBP");

InputParameters
PhysicsBasedPreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription("Physics-based preconditioner (PBP) allows individual physics to have "
                             "their own preconditioner.");

  params.addRequiredParam<std::vector<std::string>>(
      "solve_order",
      "The order the block rows will be solved in.  Put the name of variables here "
      "to stand for solving that variable's block row.  A variable may appear more "
      "than once (to create cylces if you like).");
  params.addRequiredParam<std::vector<std::string>>("preconditioner", "TODO: docstring");

  return params;
}

PhysicsBasedPreconditioner::PhysicsBasedPreconditioner(const InputParameters & params)
  : MoosePreconditioner(params),
    Preconditioner<Number>(MoosePreconditioner::_communicator),
    _nl(_fe_problem.getNonlinearSystemBase())
{
  const auto & libmesh_system = _nl.system();
  unsigned int num_systems = _nl.system().n_vars();
  _systems.resize(num_systems);
  _preconditioners.resize(num_systems);
  _off_diag.resize(num_systems);
  _off_diag_mats.resize(num_systems);
  _pre_type.resize(num_systems);

  { // Setup the Coupling Matrix so MOOSE knows what we're doing
    NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
    unsigned int n_vars = nl.nVariables();

    // The coupling matrix is held and released by FEProblemBase, so it is not released in this
    // object
    std::unique_ptr<CouplingMatrix> cm = std::make_unique<CouplingMatrix>(n_vars);

    bool full = getParam<bool>("full");

    if (!full)
    {
      // put 1s on diagonal
      for (unsigned int i = 0; i < n_vars; i++)
        (*cm)(i, i) = 1;

      // off-diagonal entries
      for (const auto & off_diag : getParam<NonlinearVariableName, NonlinearVariableName>(
               "off_diag_row", "off_diag_column"))
      {
        const auto row = libmesh_system.variable_number(off_diag.first);
        const auto column = libmesh_system.variable_number(off_diag.second);
        (*cm)(row, column) = 1;
      }

      // TODO: handle coupling entries between NL-vars and SCALAR-vars
    }
    else
    {
      for (unsigned int i = 0; i < n_vars; i++)
        for (unsigned int j = 0; j < n_vars; j++)
          (*cm)(i, j) = 1;
    }

    _fe_problem.setCouplingMatrix(std::move(cm));
  }

  // PC types
  const std::vector<std::string> & pc_types = getParam<std::vector<std::string>>("preconditioner");
  for (unsigned int i = 0; i < num_systems; i++)
    _pre_type[i] = Utility::string_to_enum<PreconditionerType>(pc_types[i]);

  // solve order
  const std::vector<std::string> & solve_order = getParam<std::vector<std::string>>("solve_order");
  _solve_order.resize(solve_order.size());
  for (const auto i : index_range(solve_order))
    _solve_order[i] = _nl.system().variable_number(solve_order[i]);

  // diag and off-diag systems
  unsigned int n_vars = _nl.system().n_vars();

  // off-diagonal entries
  const std::vector<NonlinearVariableName> & odr =
      getParam<std::vector<NonlinearVariableName>>("off_diag_row");
  const std::vector<NonlinearVariableName> & odc =
      getParam<std::vector<NonlinearVariableName>>("off_diag_column");
  std::vector<std::vector<unsigned int>> off_diag(n_vars);
  for (const auto i : index_range(odr))
  {
    unsigned int row = _nl.system().variable_number(odr[i]);
    unsigned int column = _nl.system().variable_number(odc[i]);
    off_diag[row].push_back(column);
  }
  // Add all of the preconditioning systems
  for (unsigned int var = 0; var < n_vars; var++)
    addSystem(var, off_diag[var], _pre_type[var]);

  _nl.attachPreconditioner(this);

  if (_fe_problem.solverParams()._type != Moose::ST_JFNK)
    mooseError("PBP must be used with JFNK solve type");
}

PhysicsBasedPreconditioner::~PhysicsBasedPreconditioner() { this->clear(); }

void
PhysicsBasedPreconditioner::addSystem(unsigned int var,
                                      std::vector<unsigned int> off_diag,
                                      PreconditionerType type)
{
  std::string var_name = _nl.system().variable_name(var);

  LinearImplicitSystem & precond_system =
      _fe_problem.es().add_system<LinearImplicitSystem>(var_name + "_system");
  precond_system.assemble_before_solve = false;

  const std::set<SubdomainID> * active_subdomains = _nl.getVariableBlocks(var);
  precond_system.add_variable(
      var_name + "_prec", _nl.system().variable(var).type(), active_subdomains);

  _systems[var] = &precond_system;
  _pre_type[var] = type;

  _off_diag_mats[var].resize(off_diag.size());
  for (const auto i : index_range(off_diag))
  {
    // Add the matrix to hold the off-diagonal piece
    _off_diag_mats[var][i] = &precond_system.add_matrix(_nl.system().variable_name(off_diag[i]));
  }

  _off_diag[var] = off_diag;
}

void
PhysicsBasedPreconditioner::init()
{
  TIME_SECTION("init", 2, "Initializing PhysicsBasedPreconditioner");

  // Tell libMesh that this is initialized!
  _is_initialized = true;

  const unsigned int num_systems = _systems.size();

  // If no order was specified, just solve them in increasing order
  if (_solve_order.size() == 0)
  {
    _solve_order.resize(num_systems);
    for (unsigned int i = 0; i < num_systems; i++)
      _solve_order[i] = i;
  }

  // Loop over variables
  for (unsigned int system_var = 0; system_var < num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    if (!_preconditioners[system_var])
      _preconditioners[system_var] =
          Preconditioner<Number>::build_preconditioner(MoosePreconditioner::_communicator);

    // we have to explicitly set the matrix in the preconditioner, because h-adaptivity could have
    // changed it and we have to work with the current one
    Preconditioner<Number> * preconditioner = _preconditioners[system_var].get();
    preconditioner->set_matrix(u_system.get_system_matrix());
    preconditioner->set_type(_pre_type[system_var]);

    preconditioner->init();
  }
}

void
PhysicsBasedPreconditioner::setup()
{
  const unsigned int num_systems = _systems.size();

  std::vector<JacobianBlock *> blocks;

  // Loop over variables
  for (unsigned int system_var = 0; system_var < num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    {
      JacobianBlock * block =
          new JacobianBlock(u_system, u_system.get_system_matrix(), system_var, system_var);
      blocks.push_back(block);
    }

    for (const auto diag : index_range(_off_diag[system_var]))
    {
      unsigned int coupled_var = _off_diag[system_var][diag];
      std::string coupled_name = _nl.system().variable_name(coupled_var);

      JacobianBlock * block =
          new JacobianBlock(u_system, *_off_diag_mats[system_var][diag], system_var, coupled_var);
      blocks.push_back(block);
    }
  }

  _fe_problem.computeJacobianBlocks(blocks);

  // cleanup
  for (auto & block : blocks)
    delete block;
}

void
PhysicsBasedPreconditioner::apply(const NumericVector<Number> & x, NumericVector<Number> & y)
{
  TIME_SECTION("apply", 1, "Applying PhysicsBasedPreconditioner");

  const unsigned int num_systems = _systems.size();

  MooseMesh & mesh = _fe_problem.mesh();

  // Zero out the solution vectors
  for (unsigned int sys = 0; sys < num_systems; sys++)
    _systems[sys]->solution->zero();

  // Loop over solve order
  for (const auto i : index_range(_solve_order))
  {
    unsigned int system_var = _solve_order[i];

    LinearImplicitSystem & u_system = *_systems[system_var];

    // Copy rhs from the big system into the small one
    MoosePreconditioner::copyVarValues(
        mesh, _nl.system().number(), system_var, x, u_system.number(), 0, *u_system.rhs);

    // Modify the RHS by subtracting off the matvecs of the solutions for the other preconditioning
    // systems with the off diagonal blocks in this system.
    for (const auto diag : index_range(_off_diag[system_var]))
    {
      unsigned int coupled_var = _off_diag[system_var][diag];
      LinearImplicitSystem & coupled_system = *_systems[coupled_var];
      SparseMatrix<Number> & off_diag = *_off_diag_mats[system_var][diag];
      NumericVector<Number> & rhs = *u_system.rhs;

      // This next bit computes rhs -= A*coupled_solution
      // It does what it does because there is no vector_mult_sub()
      rhs.close();
      rhs.scale(-1.0);
      rhs.close();
      off_diag.vector_mult_add(rhs, *coupled_system.solution);
      rhs.close();
      rhs.scale(-1.0);
      rhs.close();
    }

    // If there is no off_diag, then u_system.rhs will not be closed.
    // Thus, we need to close it right here
    if (!_off_diag[system_var].size())
      u_system.rhs->close();

    // Apply the preconditioner to the small system
    _preconditioners[system_var]->apply(*u_system.rhs, *u_system.solution);

    // Copy solution from small system into the big one
    // copyVarValues(mesh,system,0,*u_system.solution,0,system_var,y);
  }

  // Copy the solutions out
  for (unsigned int system_var = 0; system_var < num_systems; system_var++)
  {
    LinearImplicitSystem & u_system = *_systems[system_var];

    MoosePreconditioner::copyVarValues(
        mesh, u_system.number(), 0, *u_system.solution, _nl.system().number(), system_var, y);
  }

  y.close();
}

void
PhysicsBasedPreconditioner::clear()
{
}
