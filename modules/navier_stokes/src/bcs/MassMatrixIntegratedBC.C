//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassMatrixIntegratedBC.h"

registerMooseObject("NavierStokesApp", MassMatrixIntegratedBC);

InputParameters
MassMatrixIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
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

MassMatrixIntegratedBC::MassMatrixIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _density(getParam<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

Real
MassMatrixIntegratedBC::computeQpResidual()
{
  mooseError("should never be called");
}

Real
MassMatrixIntegratedBC::computeQpJacobian()
{
  return _test[_i][_qp] * _density * _phi[_j][_qp];
}
