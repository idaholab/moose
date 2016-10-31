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

#include "FEProblem.h"
#include "Assembly.h"


template<>
InputParameters validParams<FEProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  return params;
}

FEProblem::FEProblem(const InputParameters & parameters) :
    FEProblemBase(parameters),
    _use_nonlinear(getParam<bool>("use_nonlinear")),
    _nl_sys(_use_nonlinear ? (new NonlinearSystem(*this, "nl0")) : (new MooseEigenSystem(*this, "eigen0")))
{
  _nl = _nl_sys;
  _aux = new AuxiliarySystem(*this, "aux0");

  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(*_nl, couplingMatrix(), i);

  unsigned int dimNullSpace = parameters.get<unsigned int>("null_space_dimension");
  unsigned int dimTransposeNullSpace = parameters.get<unsigned int>("transpose_null_space_dimension");
  unsigned int dimNearNullSpace = parameters.get<unsigned int>("near_null_space_dimension");
  for (unsigned int i = 0; i < dimNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace builder might march over all nodes
    _nl->addVector("NullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NullSpace"] = dimNullSpace;
  for (unsigned int i = 0; i < dimTransposeNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace builder might march over all nodes
    _nl->addVector("TransposeNullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["TransposeNullSpace"] = dimTransposeNullSpace;
  for (unsigned int i = 0; i < dimNearNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near-nullspace builder might march over all semilocal nodes
    _nl->addVector("NearNullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NearNullSpace"] = dimNearNullSpace;

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;
}

FEProblem::~FEProblem()
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
FEProblem::setInputParametersFEProblem(InputParameters & parameters)
{
  // set _fe_problem
  FEProblemBase::setInputParametersFEProblem(parameters);
  // set _fe_problem
  parameters.set<FEProblem *>("_fe_problem") = this;
}
