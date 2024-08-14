//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Kernel.h"
#include "MassMatrix.h"
#include "MaterialProperty.h"
#include "MooseError.h"
#include "Registry.h"

registerMooseObject("MooseApp", MassMatrix);

InputParameters
MassMatrix::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Computes a finite element mass matrix");
  params.addParam<MaterialPropertyName>("density", 1, "The material property defining the density");
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");
  params.set<bool>("matrix_only") = true;
  return params;
}

MassMatrix::MassMatrix(const InputParameters & parameters)
  : Kernel(parameters), _density(getMaterialProperty<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

Real
MassMatrix::computeQpResidual()
{
  mooseError("Residual should not be calculated for the MassMatrix kernel");
}

Real
MassMatrix::computeQpJacobian()
{
  return _test[_i][_qp] * _density[_qp] * _phi[_j][_qp];
}
