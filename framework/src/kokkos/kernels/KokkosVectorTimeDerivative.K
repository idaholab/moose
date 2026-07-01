//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorTimeDerivative.h"

registerKokkosResidualObject("MooseApp", KokkosVectorTimeDerivative);

InputParameters
KokkosVectorTimeDerivative::validParams()
{
  InputParameters params = Moose::Kokkos::VectorTimeKernel::validParams();
  params.addClassDescription("The time derivative operator with the weak form of $(\\vec{\\psi_i}, "
                             "\\frac{\\partial \\vec{u_h}}{\\partial t})$.");
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");

  return params;
}

KokkosVectorTimeDerivative::KokkosVectorTimeDerivative(const InputParameters & parameters)
  : Moose::Kokkos::VectorTimeKernel(parameters), _lumping(getParam<bool>("lumping"))
{
}
