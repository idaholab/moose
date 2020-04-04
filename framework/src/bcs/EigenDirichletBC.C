//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenDirichletBC.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseApp", EigenDirichletBC);

InputParameters
EigenDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "eigen";
  params.set<MultiMooseEnum>("matrix_tags") = "eigen";

  params.addClassDescription("Dirichlet BC for eigenvalue solvers");

  return params;
}

EigenDirichletBC::EigenDirichletBC(const InputParameters & parameters) : NodalBC(parameters) {}

Real
EigenDirichletBC::computeQpResidual()
{
  return 0.0;
}

Real
EigenDirichletBC::computeQpJacobian()
{
  return 0.0;
}

Real
EigenDirichletBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}
