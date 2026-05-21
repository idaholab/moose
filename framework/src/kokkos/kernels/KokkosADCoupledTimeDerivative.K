//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADCoupledTimeDerivative.h"

registerKokkosADResidualObject("MooseApp", KokkosADCoupledTimeDerivative);

InputParameters
KokkosADCoupledTimeDerivative::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable. Weak form: "
                             "$(\\psi_i, \\frac{\\partial v_h}{\\partial t})$.");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

KokkosADCoupledTimeDerivative::KokkosADCoupledTimeDerivative(const InputParameters & parameters)
  : ADKernel(parameters), _v_dot(kokkosADCoupledDot("v"))
{
}
