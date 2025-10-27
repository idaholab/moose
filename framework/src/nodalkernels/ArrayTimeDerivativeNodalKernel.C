//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayTimeDerivativeNodalKernel.h"

registerMooseObject("MooseApp", ArrayTimeDerivativeNodalKernel);

InputParameters
ArrayTimeDerivativeNodalKernel::validParams()
{
  InputParameters params = ArrayNodalKernel::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addClassDescription("Forms the contribution to the residual and jacobian of the time "
                             "derivative term  for an array variable for ODEs "
                             "being solved at all nodes.");
  return params;
}

ArrayTimeDerivativeNodalKernel::ArrayTimeDerivativeNodalKernel(const InputParameters & parameters)
  : ArrayNodalKernel(parameters), _u_dot(_var.dofValuesDot()), _du_dot_du(_var.dofValuesDuDotDu())
{
}

void
ArrayTimeDerivativeNodalKernel::computeQpResidual(RealEigenVector & residual)
{
  residual = _u_dot[_qp];
}

RealEigenVector
ArrayTimeDerivativeNodalKernel::computeQpJacobian()
{
  return RealEigenVector::Constant(_count, _du_dot_du[_qp]);
}
