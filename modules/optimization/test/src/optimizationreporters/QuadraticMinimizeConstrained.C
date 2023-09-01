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
  InputParameters params = OptimizationReporter::validParams();
  params.addRequiredParam<Real>("objective", "Desired value of objective function.");
  params.addRequiredParam<std::vector<Real>>("solution", "Desired solution to optimization.");
  return params;
}

QuadraticMinimizeConstrained::QuadraticMinimizeConstrained(const InputParameters & parameters)
  : OptimizationReporter(parameters),
    _result(getParam<Real>("objective")),
    _solution(getParam<std::vector<Real>>("solution"))
{
  setICsandBounds();
  if (_solution.size() != _ndof)
    paramError("solution", "Size not equal to number of degrees of freedom (", _ndof, ").");
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
    Real equality_constraint = 0;
    for (const auto & val : *param)
    {
      equality_constraint += val;
    }
    eqs_constraints.set(i++, equality_constraint - 5);
  }
  eqs_constraints.close();
}
void
QuadraticMinimizeConstrained::computeEqualityGradient(libMesh::PetscMatrix<Number> & gradient) const
{
  gradient.zero();

  for (unsigned long i = 0; i < _n_eq_cons; i++)
    for (unsigned long j = 0; j < _parameters[0]->size(); j++)
      gradient.set(i, j, 1);

  gradient.close();
}
