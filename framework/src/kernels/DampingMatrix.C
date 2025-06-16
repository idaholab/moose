//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Kernel.h"
#include "DampingMatrix.h"
#include "MaterialProperty.h"
#include "MooseError.h"
#include "Registry.h"

registerMooseObject("MooseApp", DampingMatrix);

InputParameters
DampingMatrix::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Computes a finite element damping matrix");
  params.addParam<MaterialPropertyName>("density", 1, "The material property defining the density");
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");
  params.set<bool>("matrix_only") = true;
  return params;
}

DampingMatrix::DampingMatrix(const InputParameters & parameters)
  : Kernel(parameters), _density(getMaterialProperty<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

Real
DampingMatrix::computeQpResidual()
{
  mooseError("Residual should not be calculated for the DampingMatrix kernel");
}

Real
DampingMatrix::computeQpJacobian()
{
  // _console << "density: "<<_density[_qp] << std::endl;
  return _test[_i][_qp] * _density[_qp] * _phi[_j][_qp];
}
