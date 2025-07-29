//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectiveFluxBC.h"

registerMooseObject("MooseApp", KokkosConvectiveFluxBC);

InputParameters
KokkosConvectiveFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.set<Real>("rate") = 7500;
  params.set<Real>("initial") = 500;
  params.set<Real>("final") = 500;
  params.set<Real>("duration") = 0.0;
  params.addClassDescription(
      "Determines boundary values via the initial and final values, flux, and exposure duration");
  return params;
}

KokkosConvectiveFluxBC::KokkosConvectiveFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _rate(getParam<Real>("rate")),
    _duration(getParam<Real>("duration"))
{
}
