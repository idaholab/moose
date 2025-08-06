//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVacuumBC.h"

registerMooseObject("MooseApp", KokkosVacuumBC);

InputParameters
KokkosVacuumBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Vacuum boundary condition for diffusion.");
  params.addParam<Real>("alpha", 1, "Diffusion coefficient.");
  return params;
}

KokkosVacuumBC::KokkosVacuumBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _alpha(getParam<Real>("alpha"))
{
}
