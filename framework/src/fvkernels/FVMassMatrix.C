//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMassMatrix.h"
#include "Function.h"

registerMooseObject("MooseApp", FVMassMatrix);

InputParameters
FVMassMatrix::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Computes a 'mass matrix', which will just be a diagonal matrix for the finite volume "
      "method, meant for use in preconditioning schemes which require one");
  params.addParam<MooseFunctorName>("density", 1.0, "Optional density weighting functor");
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");
  params.set<bool>("matrix_only") = true;
  return params;
}

FVMassMatrix::FVMassMatrix(const InputParameters & parameters)
  : FVElementalKernel(parameters), _density(getFunctor<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

void
FVMassMatrix::computeResidual()
{
}

ADReal
FVMassMatrix::computeQpResidual()
{
  const auto elem = makeElemArg(_current_elem);
  const auto state = determineState();
  return _density(elem, state) * _var(elem, state);
}
