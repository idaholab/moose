//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantConvectiveBC.h"

registerKokkosResidualObject("KokkosTrainingApp", KokkosConstantConvectiveBC);

InputParameters
KokkosConstantConvectiveBC::validParams()
{
  InputParameters params = KokkosConvectiveBCBase::validParams();
  params.addClassDescription("Convective flux h (u - u_inf) with a constant coefficient h.");
  params.addRequiredParam<Real>("h", "Heat transfer coefficient");
  params.declareControllable("h");
  return params;
}

KokkosConstantConvectiveBC::KokkosConstantConvectiveBC(const InputParameters & parameters)
  : KokkosConvectiveBCBase(parameters), _h(getParam<Real>("h"))
{
}
