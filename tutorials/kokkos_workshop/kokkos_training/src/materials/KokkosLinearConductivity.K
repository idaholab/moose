//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearConductivity.h"

registerKokkosMaterial("KokkosTrainingApp", KokkosLinearConductivity);

InputParameters
KokkosLinearConductivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Conductivity k = k0 (1 + beta T) and its derivative with respect "
                             "to T as an on-demand property.");
  params.addRequiredCoupledVar("temperature", "The variable the conductivity depends on");
  params.addParam<Real>("k0", 1.0, "Conductivity at T = 0");
  params.addParam<Real>("beta", 0.0, "Linear coefficient of the conductivity in T");
  return params;
}

KokkosLinearConductivity::KokkosLinearConductivity(const InputParameters & parameters)
  : Material(parameters),
    _T(kokkosCoupledValue("temperature")),
    _k0(getParam<Real>("k0")),
    _beta(getParam<Real>("beta")),
    _k(declareKokkosProperty<Real>("conductivity")),
    _dk_dT(declareKokkosOnDemandProperty<Real>("conductivity_derivative"))
{
}
