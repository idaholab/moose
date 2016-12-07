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

#include "EigenProblem.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

template<>
InputParameters validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  params.addParam<unsigned int>("n_eigen_pairs", 1, "The dimension of the nullspace");
  params.addParam<unsigned int>("n_basis_vectors", 3, "The dimension of the nullspace");
  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters) :
    FEProblemBase(parameters),
    _n_eigen_pairs_required(getParam<unsigned int>("n_eigen_pairs")),
    _nl_eigen(new NonlinearEigenSystem(*this, "eigen0"))
{
#if LIBMESH_HAVE_SLEPC
  _nl = _nl_eigen;
  _aux = new AuxiliarySystem(*this, "aux0");

  // Set necessary parametrs used in EigenSystem::solve(),
  // i.e. the number of requested eigenpairs nev and the number
  // of basis vectors ncv used in the solution algorithm. Note that
  // ncv >= nev must hold and ncv >= 2*nev is recommended.
  es().parameters.set<unsigned int>("eigenpairs")    = _n_eigen_pairs_required;
  es().parameters.set<unsigned int>("basis vectors") = getParam<unsigned int>("n_basis_vectors");

  newAssemblyArray(*_nl_eigen);

  FEProblemBase::initNullSpaceVectors(parameters, *_nl_eigen);
#else
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure\n");
#endif /* LIBMESH_HAVE_SLEPC */
}

EigenProblem::~EigenProblem()
{
#if LIBMESH_HAVE_SLEPC
  FEProblemBase::deleteAssemblyArray();
#endif /* LIBMESH_HAVE_SLEPC */
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
  {
     _nl->solve();
     _nl->update();
  }

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  Moose::perf_log.pop("Eigen_solve()", "Execution");
}

#if LIBMESH_HAVE_SLEPC
bool
EigenProblem::converged()
{
  return _nl_eigen->converged();
}
#endif
