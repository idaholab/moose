//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosScalarVarBC.h"

registerKokkosResidualObject("MooseTestApp", KokkosScalarVarBC);

InputParameters
KokkosScalarVarBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("alpha", "The scalar variable coupled in");
  return params;
}

KokkosScalarVarBC::KokkosScalarVarBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _alpha_var(coupledScalar("alpha")),
    _alpha(kokkosCoupledScalarValue("alpha"))
{
}
