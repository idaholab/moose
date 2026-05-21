//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADTimeKernel.h"

namespace Moose::Kokkos
{

InputParameters
ADTimeKernel::validParams()
{
  InputParameters params = ADKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

ADTimeKernel::ADTimeKernel(const InputParameters & parameters)
  : ADKernel(parameters), _u_dot(_var, Moose::SOLUTION_DOT_TAG)
{
}

} // namespace Moose::Kokkos
