#include "HeatStructureEnergyBase.h"
#include "HeatConductionModel.h"

template <>
InputParameters
validParams<HeatStructureEnergyBase>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();

  params.addClassDescription("Computes the total energy for a heat structure.");

  params.addParam<unsigned int>("n_units", 1, "Number of units of heat structure represents");

  return params;
}

HeatStructureEnergyBase::HeatStructureEnergyBase(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _n_units(getParam<unsigned int>("n_units")),
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
  return _rho[_qp] * _cp[_qp] * _T[_qp] * _n_units;
}
