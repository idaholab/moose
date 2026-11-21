//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledBEKinetic.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosCoupledBEKinetic);

InputParameters
KokkosCoupledBEKinetic::validParams()
{
  InputParameters params = TimeKernelValue::validParams();
  params.addRequiredParam<std::vector<Real>>("weight",
                                             "The weight of kinetic species concentration");
  params.addCoupledVar("v", "List of kinetic species being coupled by concentration");
  params.addClassDescription("Derivative of kinetic species concentration wrt time");
  return params;
}

KokkosCoupledBEKinetic::KokkosCoupledBEKinetic(const InputParameters & parameters)
  : TimeKernelValue(parameters),
    _porosity(getKokkosMaterialProperty<Real>("porosity")),
    _weight(getParam<std::vector<Real>>("weight")),
    _n(coupledComponents("v")),
    _vals(kokkosCoupledValues("v")),
    _vals_old(kokkosCoupledValuesOld("v"))
{
}
