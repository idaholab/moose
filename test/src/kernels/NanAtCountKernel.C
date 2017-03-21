/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
