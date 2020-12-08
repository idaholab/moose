#include "ObjectiveMinimize.h"

registerMooseObject("isopodApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addParam<ReporterName>("data_computed",
                                "Name of reporter value containing measure pointed values.");
  params.addParam<ReporterValueName>("data_computed_name",
                                     "Reporter value to create if 'data_computed' does not exist.");
  params.addRequiredParam<std::vector<Real>>("data_target", "Target measured value.");
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _data_computed(getDataValueHelper("data_computed", "data_computed_name")),
    _data_target(getParam<std::vector<Real>>("data_target"))
{
}

Real
ObjectiveMinimize::computeObjective()
{
  if (_data_computed.size() != _data_target.size())
    paramError("data_target",
               "Computed data is inconsistent with target data.  \ndata_target.size()= ",
               _data_target.size(),
               "\ndata_computed.size()= ",
               _data_computed.size());

  Real val = 0;
  for (std::size_t i = 0; i < _data_computed.size(); ++i)
  {
    Real tmp = _data_computed[i] - _data_target[i];
    val += tmp * tmp;
  }
  val = 0.5 * val;
  return val;
}
