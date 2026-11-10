//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    _u_dot(*_vars[0], Moose::SOLUTION_DOT_TAG),
    _du_dot_du(_vars[0]->sys().duDotDu(_vars[0]->number()))
{
}

} // namespace Moose::Kokkos
