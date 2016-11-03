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
#include "Assembly.h"

template<>
InputParameters validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblem>();
  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters) :
    FEProblem(parameters)
{
  _nl =  new NonlinearEigenSystem(*this, "eigen0");
  _aux = new AuxiliarySystem(*this, "aux0");

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

#endif /* LIBMESH_HAVE_SLEPC */
