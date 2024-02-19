//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadraticMinimizeConstrained.h"
#include "libmesh/petsc_vector.h"

registerMooseObject("OptimizationTestApp", QuadraticMinimizeConstrained);

InputParameters
QuadraticMinimizeConstrained::validParams()
{
  InputParameters params = QuadraticMinimize::validParams();
  params.addRequiredParam<Real>("solution_sum_equality",
                                "Desired sum of the solution for constrained optimization.");
  return params;
}

QuadraticMinimizeConstrained::QuadraticMinimizeConstrained(const InputParameters & parameters)
  : QuadraticMinimize(parameters),
    _result(getParam<Real>("objective")),
    _solution(getParam<std::vector<Real>>("solution")),
    _eq_constraint(getParam<Real>("solution_sum_equality"))
{
}

Real
QuadraticMinimizeConstrained::computeObjective()
{
  Real obj = _result;
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      Real tmp = val - _solution[i++];
      obj += tmp * tmp;
    }

  return obj;
}

void
QuadraticMinimizeConstrained::computeGradient(libMesh::PetscVector<Number> & gradient) const
{
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      gradient.set(i, 2.0 * (val - _solution[i]));
      i++;
    }
  gradient.close();
}
void
QuadraticMinimizeConstrained::computeEqualityConstraints(
    libMesh::PetscVector<Number> & eqs_constraints) const
{

  unsigned int i = 0;
  for (const auto & param : _parameters)
  {
    const Real equality_constraint = std::accumulate(param->begin(), param->end(), 0.0);
    eqs_constraints.set(i++, equality_constraint - _eq_constraint);
  }
  eqs_constraints.close();
}
void
QuadraticMinimizeConstrained::computeEqualityGradient(libMesh::PetscMatrix<Number> & gradient) const
{
  gradient.zero();
  for (const auto i : make_range(_n_eq_cons))
    for (const auto j : index_range(*_parameters[0]))
      gradient.set(i, j, 1);

  gradient.close();
}
