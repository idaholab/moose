//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorTimeKernel.h"

namespace Moose::Kokkos
{

InputParameters
VectorTimeKernel::validParams()
{
  InputParameters params = VectorKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

VectorTimeKernel::VectorTimeKernel(const InputParameters & parameters)
  : VectorKernel(parameters),
    _u_dot(_var, Moose::SOLUTION_DOT_TAG),
    _du_dot_du(_var.sys().duDotDu(_var.number()))
{
}

} // namespace Moose::Kokkos
