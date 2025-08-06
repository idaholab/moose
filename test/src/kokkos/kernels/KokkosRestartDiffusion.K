//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosRestartDiffusion.h"

registerMooseObject("MooseTestApp", KokkosRestartDiffusion);

InputParameters
KokkosRestartDiffusion::validParams()
{
  InputParameters params = KokkosDiffusion::validParams();
  return params;
}

KokkosRestartDiffusion::KokkosRestartDiffusion(const InputParameters & parameters)
  : KokkosDiffusion(parameters),
    _step(declareRestartableData<unsigned int>("step", 0)),
    _coef(declareRestartableData<Moose::Kokkos::Array<Real>>("coef", 100))
{
  *_coef = 1;
}

void
KokkosRestartDiffusion::timestepSetup()
{
  _step = _step + 1;
  _coef(_step) = _coef(_step - 1) + 1;
  _coef->copyToDevice();
}
