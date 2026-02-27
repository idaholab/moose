//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTimeKernel.h"

namespace Moose::Kokkos
{

InputParameters
TimeKernel::validParams()
{
  InputParameters params = Kernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

TimeKernel::TimeKernel(const InputParameters & parameters)
  : Kernel(parameters), _u_dot(variables(), Moose::SOLUTION_DOT_TAG), _du_dot_du(variables().size())
{
}

TimeKernel::TimeKernel(const TimeKernel & object)
  : Kernel(object), _u_dot(object._u_dot), _du_dot_du(object._du_dot_du)
{
  // TODO: We want to use Array<Scalar<const Real>> for _du_dot_du so that it is automatically
  // synchronized, but currently there is an NVCC bug preventing it

  for (unsigned int i = 0; i < variables().size(); ++i)
    _du_dot_du[i] = variables()[i]->sys().duDotDu(variables()[i]->number());

  _du_dot_du.copyToDevice();
}

} // namespace Moose::Kokkos
