//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenArrayDirichletBC.h"

registerMooseObject("MooseApp", EigenArrayDirichletBC);

InputParameters
EigenArrayDirichletBC::validParams()
{
  InputParameters params = ArrayNodalBC::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "eigen";
  params.set<MultiMooseEnum>("matrix_tags") = "eigen";

  params.addClassDescription("Array Dirichlet BC for eigenvalue solvers");

  return params;
}

EigenArrayDirichletBC::EigenArrayDirichletBC(const InputParameters & parameters)
  : ArrayNodalBC(parameters)
{
}

void
EigenArrayDirichletBC::computeQpResidual(RealEigenVector & residual)
{
  residual.setZero();
}

RealEigenVector
EigenArrayDirichletBC::computeQpJacobian()
{
  return RealEigenVector::Zero(_var.count());
}

RealEigenMatrix
EigenArrayDirichletBC::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
