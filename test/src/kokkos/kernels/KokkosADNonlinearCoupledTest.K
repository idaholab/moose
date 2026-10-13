//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADNonlinearCoupledTest.h"

registerKokkosADResidualObject("MooseTestApp", KokkosADNonlinearCoupledTest);

InputParameters
KokkosADNonlinearCoupledTest::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Test kernel with nonlinear dependence on the values and gradients of "
                             "the kernel variable and a coupled variable.");
  params.addRequiredCoupledVar("v", "The coupled variable");
  return params;
}

KokkosADNonlinearCoupledTest::KokkosADNonlinearCoupledTest(const InputParameters & parameters)
  : ADKernel(parameters),
    _u(adValue()),
    _grad_u(adGradient()),
    _v(adCoupledValue("v")),
    _grad_v(adCoupledGradient("v"))
{
}
