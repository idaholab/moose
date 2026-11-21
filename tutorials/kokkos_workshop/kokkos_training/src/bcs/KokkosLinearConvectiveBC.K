//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearConvectiveBC.h"

registerKokkosResidualObject("KokkosTrainingApp", KokkosLinearConvectiveBC);

InputParameters
KokkosLinearConvectiveBC::validParams()
{
  InputParameters params = KokkosConvectiveBCBase::validParams();
  params.addClassDescription(
      "Convective flux h(u) (u - u_inf) with the coefficient h(u) = h0 (1 + gamma u).");
  params.addRequiredParam<Real>("h0", "Heat transfer coefficient at u = 0");
  params.addParam<Real>("gamma", 0.0, "Linear coefficient of the heat transfer coefficient in u");
  return params;
}

KokkosLinearConvectiveBC::KokkosLinearConvectiveBC(const InputParameters & parameters)
  : KokkosConvectiveBCBase(parameters), _h0(getParam<Real>("h0")), _gamma(getParam<Real>("gamma"))
{
}
