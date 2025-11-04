//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTimeDerivative.h"

registerKokkosResidualObject("MooseApp", KokkosTimeDerivative);

InputParameters
KokkosTimeDerivative::validParams()
{
  InputParameters params = Moose::Kokkos::TimeKernel::validParams();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
  params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                             "\\frac{\\partial u_h}{\\partial t})$.");
  return params;
}

KokkosTimeDerivative::KokkosTimeDerivative(const InputParameters & parameters)
  : Moose::Kokkos::TimeKernel(parameters), _lumping(getParam<bool>("lumping"))
{
}
