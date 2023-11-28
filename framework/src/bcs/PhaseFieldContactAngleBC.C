//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldContactAngleBC.h"

registerMooseObject("PhaseFieldApp", PhaseFieldContactAngleBC);

InputParameters
PhaseFieldContactAngleBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription("Enforce contact angle BC using phase field variable");
  params.addRequiredCoupledVar("pf", "phase field variable");
  params.addRequiredParam<Real>("epsilon", "Interface width");
  params.addRequiredParam<Real>("lambda", "Mixing energy density");
  params.addRequiredParam<Real>("sigma", "Surface tension coefficient");
  params.addRequiredParam<Real>("contactangle",
                                "Contact angle of the fluid with the wall boundary in Radians");
  return params;
}

PhaseFieldContactAngleBC::PhaseFieldContactAngleBC(const InputParameters & params)
  : ADIntegratedBC(params),
    _pf(adCoupledValue("pf")),
    _grad_pf(adCoupledGradient("pf")),
    _epsilon(getParam<Real>("epsilon")),
    _lambda(getParam<Real>("lambda")),
    _sigma(getParam<Real>("sigma")),
    _contactangle(getParam<Real>("contactangle"))
{
}

ADReal
PhaseFieldContactAngleBC::computeQpResidual()
{
  return _test[_i][_qp] * (0.75 * _epsilon * _epsilon / _lambda) * _sigma *
         std::cos(_contactangle) * (1 - _pf[_qp] * _pf[_qp]);
}
