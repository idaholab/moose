//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMassMatrix.h"

registerMooseObject("NavierStokesApp", VectorMassMatrix);

InputParameters
VectorMassMatrix::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription("Computes a finite element mass matrix meant for use in "
                             "preconditioning schemes which require one");
  params.addParam<Real>("density", 1, "Optional density for scaling the computed mass.");
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");
  params.set<bool>("matrix_only") = true;
  return params;
}

VectorMassMatrix::VectorMassMatrix(const InputParameters & parameters)
  : VectorKernel(parameters), _density(getParam<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

Real
VectorMassMatrix::computeQpResidual()
{
  mooseError("should never be called");
}

Real
VectorMassMatrix::computeQpJacobian()
{
  return _density * _phi[_j][_qp] * _test[_i][_qp];
}
