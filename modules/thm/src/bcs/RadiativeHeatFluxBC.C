#include "RadiativeHeatFluxBC.h"
#include "Function.h"

registerMooseObject("THMApp", RadiativeHeatFluxBC);

template <>
InputParameters
validParams<RadiativeHeatFluxBC>()
{
  InputParameters params = validParams<RadiativeHeatFluxBCBase>();

  params.addParam<FunctionName>("view_factor", "1", "View factor function");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

RadiativeHeatFluxBC::RadiativeHeatFluxBC(const InputParameters & parameters)
  : RadiativeHeatFluxBCBase(parameters),

    _view_factor_fn(getFunction("view_factor"))
{
}

Real
RadiativeHeatFluxBC::coefficient() const
{
  return _eps_boundary * _view_factor_fn.value(_t, _q_point[_qp]);
}
