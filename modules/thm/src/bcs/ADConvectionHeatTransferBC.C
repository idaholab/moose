#include "ADConvectionHeatTransferBC.h"
#include "Function.h"

registerMooseObject("THMApp", ADConvectionHeatTransferBC);

InputParameters
ADConvectionHeatTransferBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc_ambient",
                                        "Ambient heat transfer coefficient function");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");
  return params;
}

ADConvectionHeatTransferBC::ADConvectionHeatTransferBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc_ambient")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

ADReal
ADConvectionHeatTransferBC::computeQpResidual()
{
  return _scale_pp * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_u[_qp] - _T_ambient_fn.value(_t, _q_point[_qp])) * _test[_i][_qp];
}
