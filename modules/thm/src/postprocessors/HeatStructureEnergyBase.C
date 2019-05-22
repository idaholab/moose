#include "HeatStructureEnergyBase.h"
#include "HeatConductionModel.h"

template <>
InputParameters
validParams<HeatStructureEnergyBase>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();

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
