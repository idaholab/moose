//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosTimeKernelValue.h"

namespace Moose::Kokkos
{

InputParameters
TimeKernelValue::validParams()
{
  InputParameters params = KernelValue::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

TimeKernelValue::TimeKernelValue(const InputParameters & parameters)
  : KernelValue(parameters), _u_dot(_vars, Moose::SOLUTION_DOT_TAG), _du_dot_du(_vars.size())
{
}

TimeKernelValue::TimeKernelValue(const TimeKernelValue & object)
  : KernelValue(object), _u_dot(object._u_dot), _du_dot_du(object._du_dot_du)
{
  // TODO: We want to use Array<Scalar<const Real>> for _du_dot_du so that it is automatically
  // synchronized, but currently there is an NVCC bug preventing it

  for (unsigned int i = 0; i < _vars.size(); ++i)
    _du_dot_du[i] = _vars[i]->sys().duDotDu(_vars[i]->number());

  _du_dot_du.copyToDevice();
}

} // namespace Moose::Kokkos
