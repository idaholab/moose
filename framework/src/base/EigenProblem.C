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

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"

#include "libmesh/system.h"
#include "libmesh/eigen_solver.h"

template <>
InputParameters
validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  params.addParam<unsigned int>("n_eigen_pairs", 1, "The number of eigen pairs");
  params.addParam<unsigned int>("n_basis_vectors", 3, "The dimension of eigen subspaces");
#if LIBMESH_HAVE_SLEPC
  params += Moose::SlepcSupport::getSlepcEigenProblemValidParams();
#endif
  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    _n_eigen_pairs_required(getParam<unsigned int>("n_eigen_pairs")),
    _generalized_eigenvalue_problem(false),
    _nl_eigen(new NonlinearEigenSystem(*this, "eigen0"))
{
#if LIBMESH_HAVE_SLEPC
  _nl = _nl_eigen;
  _aux = new AuxiliarySystem(*this, "aux0");

  // Set necessary parametrs used in EigenSystem::solve(),
  // i.e. the number of requested eigenpairs nev and the number
  // of basis vectors ncv used in the solution algorithm. Note that
  // ncv >= nev must hold and ncv >= 2*nev is recommended.
  es().parameters.set<unsigned int>("eigenpairs") = _n_eigen_pairs_required;
  es().parameters.set<unsigned int>("basis vectors") = getParam<unsigned int>("n_basis_vectors");

  newAssemblyArray(*_nl_eigen);

  FEProblemBase::initNullSpaceVectors(parameters, *_nl_eigen);

  _eq.parameters.set<EigenProblem *>("_eigen_problem") = this;

  Moose::SlepcSupport::storeSlepcEigenProblemOptions(*this, _pars);

  setEigenproblemType(solverParams()._eigen_problem_type);
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

#if LIBMESH_HAVE_SLEPC
void
EigenProblem::setEigenproblemType(Moose::EigenProblemType eigen_problem_type)
{
  switch (eigen_problem_type)
  {
    case Moose::EPT_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::HEP);
      _generalized_eigenvalue_problem = false;
      break;

    case Moose::EPT_NON_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::NHEP);
      _generalized_eigenvalue_problem = false;
      break;

    case Moose::EPT_GEN_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GHEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_GEN_INDEFINITE:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GHIEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_GEN_NON_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GNHEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_POS_GEN_NON_HERMITIAN:
      mooseError("libMesh does not support EPT_POS_GEN_NON_HERMITIAN currently \n");
      break;

    default:
      mooseError("Unknown eigen solver type \n");
  }
}
#endif

void
EigenProblem::computeJacobian(const NumericVector<Number> & soln,
                              SparseMatrix<Number> & jacobian,
                              Moose::KernelType kernel_type)
{
  // to avoid computing residual
  solverParams()._type = Moose::ST_NEWTON;
  FEProblemBase::computeJacobian(soln, jacobian, kernel_type);
}

void
EigenProblem::checkProblemIntegrity()
{
  FEProblemBase::checkProblemIntegrity();
  _nl_eigen->checkIntegrity();
}

void
EigenProblem::solve()
{
  Moose::perf_log.push("Eigen_solve()", "Execution");
#if LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::slepcSetOptions(*this); // Make sure the SLEPc options are setup for this app
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

bool
EigenProblem::converged()
{
  return _nl_eigen->converged();
}
