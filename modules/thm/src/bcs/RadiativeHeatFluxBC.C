#include "RadiativeHeatFluxBC.h"
#include "Function.h"

registerMooseObject("THMApp", RadiativeHeatFluxBC);

InputParameters
RadiativeHeatFluxBC::validParams()
{
  InputParameters params = RadiativeHeatFluxBCBase::validParams();

  params.addParam<FunctionName>("view_factor", "1", "View factor function");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

RadiativeHeatFluxBC::RadiativeHeatFluxBC(const InputParameters & parameters)
  : RadiativeHeatFluxBCBase(parameters),

    _view_factor_fn(getFunction("view_factor")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

Real
RadiativeHeatFluxBC::coefficient() const
{
  return _scale_pp * _eps_boundary * _view_factor_fn.value(_t, _q_point[_qp]);
}
