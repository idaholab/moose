//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NanAtCountKernel.h"

registerMooseObject("MooseTestApp", NanAtCountKernel);

InputParameters
NanAtCountKernel::validParams()
{
  InputParameters params = Kernel::validParams();
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

void
NanAtCountKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  accumulateTaggedLocalResidual();

  // Increase this number so that we do not hit it again
  // in the next function evaluation.
  if (_count == _count_to_nan)
    _count++;
}

Real
NanAtCountKernel::computeQpResidual()
{
  if (_count == _count_to_nan)
    return std::numeric_limits<Real>::infinity();

  return 0;
}

void
NanAtCountKernel::computeJacobian()
{
  _count++;

  if (_print_count)
    _console << "NanAtCountKernel " << name() << " count: " << _count << std::endl;
}
