//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVolumeFractionAbove.h"

registerKokkosUserObject("KokkosTrainingApp", KokkosVolumeFractionAbove);

InputParameters
KokkosVolumeFractionAbove::validParams()
{
  InputParameters params = KokkosElementVariablePostprocessor::validParams();
  params.addClassDescription(
      "Fraction of the domain volume where the variable exceeds a threshold.");
  params.addRequiredParam<Real>("threshold", "The variable value above which volume is counted");
  return params;
}

KokkosVolumeFractionAbove::KokkosVolumeFractionAbove(const InputParameters & parameters)
  : KokkosElementVariablePostprocessor(parameters), _threshold(getParam<Real>("threshold"))
{
}

void
KokkosVolumeFractionAbove::initialize()
{
  allocateReductionBuffer(2);
}

void
KokkosVolumeFractionAbove::finalize()
{
  gatherSum(_reduction_buffer[0]);
  gatherSum(_reduction_buffer[1]);
}

Real
KokkosVolumeFractionAbove::getValue() const
{
  return _reduction_buffer[0] / _reduction_buffer[1];
}
