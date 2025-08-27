//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWENumericalFluxBase.h"

InputParameters
SWENumericalFluxBase::validParams()
{
  InputParameters params = InternalSideFluxBase::validParams();
  params.addClassDescription(
      "Base for shallow-water numerical fluxes (SWE) with shared parameters.");
  params.addParam<Real>("gravity", 9.81, "Gravitational acceleration g");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  return params;
}

SWENumericalFluxBase::SWENumericalFluxBase(const InputParameters & parameters)
  : InternalSideFluxBase(parameters),
    _g(getParam<Real>("gravity")),
    _h_eps(getParam<Real>("dry_depth"))
{
}
