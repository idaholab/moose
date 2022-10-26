//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DissociationFluxBC.h"

registerMooseObject("ScalarTransportApp", DissociationFluxBC);

InputParameters
DissociationFluxBC::validParams()
{
  auto params = ADIntegratedBC::validParams();
  params.addRequiredCoupledVar("v",
                               "The variable that is dissociating on this boundary to "
                               "form the mobile species (specified with the variable param)");
  params.addParam<Real>("Kd", 1, "The dissociation coefficient");
  params.addClassDescription("Models creation of the variable at boundaries due to dissociation of "
                             "a coupled variable, e.g. B -> A");
  return params;
}

DissociationFluxBC::DissociationFluxBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _v(adCoupledValue("v")), _Kd(getParam<Real>("Kd"))
{
}

ADReal
DissociationFluxBC::computeQpResidual()
{
  return -_test[_i][_qp] * _Kd * _v[_qp];
}
