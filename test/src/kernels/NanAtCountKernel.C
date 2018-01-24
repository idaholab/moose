//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NanAtCountKernel.h"

template <>
InputParameters
validParams<NanAtCountKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("count", "Will return a Nan during this residual count");
  params.addParam<bool>("print_count", false, "Print out the count.");

  return params;
}

NanAtCountKernel::NanAtCountKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _count_to_nan(getParam<unsigned int>("count")),
    _print_count(getParam<bool>("print_count")),
    _count(0)
{
}

Real
NanAtCountKernel::computeQpResidual()
{
  _count++;

  if (_print_count)
    _console << "NanAtCountKernel " << name() << " count: " << _count << "\n";

  if (_count == _count_to_nan)
    return std::numeric_limits<Real>::infinity();

  return 0;
}
