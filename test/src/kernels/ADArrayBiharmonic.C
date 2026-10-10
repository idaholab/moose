//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayBiharmonic.h"

registerMooseObject("MooseTestApp", ADArrayBiharmonic);

InputParameters
ADArrayBiharmonic::validParams()
{
  InputParameters params = ADArrayKernel::validParams();
  params.addCoupledVar("v", "The coupled array variable; the kernel variable if not given");
  params.addClassDescription("Adds the biharmonic residual $(\\nabla^2 \\vec{v}, \\nabla^2 "
                             "\\vec{u}^\\ast)$ using automatic differentiation.");
  return params;
}

ADArrayBiharmonic::ADArrayBiharmonic(const InputParameters & parameters)
  : ADArrayKernel(parameters),
    _second_v(isCoupled("v") ? adCoupledArraySecond("v") : _var.adSecondSln()),
    _second_test(secondTest())
{
}

void
ADArrayBiharmonic::computeQpResidual(ADRealEigenVector & residual)
{
  // the Laplacian is the trace of the Hessian, whose entry (d, d) is column 3 * d + d
  ADRealEigenVector laplacian = _second_v[_qp].col(0);
  for (const auto d : make_range(1u, unsigned(Moose::dim)))
    laplacian += _second_v[_qp].col(Moose::dim * d + d);
  residual = laplacian * _second_test[_i][_qp].tr();
}
