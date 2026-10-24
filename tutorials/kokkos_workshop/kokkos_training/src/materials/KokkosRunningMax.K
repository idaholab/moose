//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosRunningMax.h"

registerKokkosMaterial("KokkosTrainingApp", KokkosRunningMax);

InputParameters
KokkosRunningMax::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Running maximum over time of a coupled variable, stored in the "
                             "stateful property 'running_max'.");
  params.addRequiredCoupledVar("v", "The variable whose maximum is tracked");
  return params;
}

KokkosRunningMax::KokkosRunningMax(const InputParameters & parameters)
  : Material(parameters),
    _v(kokkosCoupledValue("v")),
    _max(declareKokkosProperty<Real>("running_max")),
    _max_old(getKokkosMaterialPropertyOld<Real>("running_max"))
{
}
