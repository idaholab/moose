//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureEnergyBase.h"
#include "HeatConductionModel.h"

InputParameters
HeatStructureEnergyBase::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addClassDescription("Computes the total energy for a heat structure.");

  params.addParam<Real>("n_units", 1., "Number of units of heat structure represents");
  params.addParam<Real>("T_ref", 0, "Reference temperature");

  return params;
}

HeatStructureEnergyBase::HeatStructureEnergyBase(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _n_units(getParam<Real>("n_units")),
    _T_ref(getParam<Real>("T_ref")),
    _rho(getMaterialPropertyByName<Real>(HeatConductionModel::DENSITY)),
    _cp(getMaterialPropertyByName<Real>(HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE)),
    _T_var(&_fe_problem.getStandardVariable(_tid, HeatConductionModel::TEMPERATURE)),
    _T(_T_var->sln())
{
  addMooseVariableDependency(_T_var);
}

Real
HeatStructureEnergyBase::computeQpIntegral()
{
  return _rho[_qp] * _cp[_qp] * (_T[_qp] - _T_ref) * _n_units;
}
