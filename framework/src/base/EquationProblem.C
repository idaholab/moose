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

#include "EquationProblem.h"
#include "Assembly.h"

template<>
InputParameters validParams<EquationProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addParam<bool>("use_nonlinear", true, "Determines whether to use a Nonlinear vs a Eigenvalue system (Automatically determined based on executioner)");
  return params;
}

EquationProblem::EquationProblem(const InputParameters & parameters) :
    FEProblem(parameters),
    _use_nonlinear(getParam<bool>("use_nonlinear"))
{
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
    _nl.addVector("TransposeNullSpace" + oss.str(), false, GHOSTED);
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
}

EquationProblem::~EquationProblem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _assembly[i];
  }
}
