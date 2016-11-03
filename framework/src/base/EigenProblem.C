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

#include "EigenProblem.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

template<>
InputParameters validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addParam<unsigned int>("n_eigen_pairs", 1, "The dimension of the nullspace");
  params.addParam<unsigned int>("n_basis_vectors", 3, "The dimension of the nullspace");
  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters) :
    FEProblem(parameters),
    _nl_eigen(new NonlinearEigenSystem(*this, "eigen0"))
{
  _nl =  _nl_eigen;
  _aux = new AuxiliarySystem(*this, "aux0");

  // Set necessary parametrs used in EigenSystem::solve(),
  // i.e. the number of requested eigenpairs nev and the number
  // of basis vectors ncv used in the solution algorithm. Note that
  // ncv >= nev must hold and ncv >= 2*nev is recommended.
  es().parameters.set<unsigned int>("eigenpairs")    = getParam<unsigned int>("n_eigen_pairs");
  es().parameters.set<unsigned int>("basis vectors") = getParam<unsigned int>("n_basis_vectors");

  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(*_nl, couplingMatrix(), i);

  unsigned int dimNullSpace      = parameters.get<unsigned int>("dimNullSpace");
  unsigned int dimNearNullSpace  = parameters.get<unsigned int>("dimNearNullSpace");
  for (unsigned int i = 0; i < dimNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace builder might march over all nodes
    _nl->addVector("NullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NullSpace"] = dimNullSpace;
  for (unsigned int i = 0; i < dimNearNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near-nullspace builder might march over all semilocal nodes
    _nl->addVector("NearNullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NearNullSpace"] = dimNearNullSpace;
}

EigenProblem::~EigenProblem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _assembly[i];
  }

  delete _nl;

  delete _aux;
}



void
EigenProblem::solve()
{
  Moose::perf_log.push("Eigen_solve()", "Execution");
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(*this); // Make sure the PETSc options are setup for this app
#endif

  if (_solve)
    _nl->solve();

  if (_solve)
    _nl->update();

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  Moose::perf_log.pop("Eigen_solve()", "Execution");
}


bool
EigenProblem::converged()
{
  _console<<"WARNING: did not implement yet \n";
  return true;
}


void
EigenProblem::outputStep(ExecFlagType /*type*/)
{
  _nl->update();
  _aux->update();
}



#endif /* LIBMESH_HAVE_SLEPC */
