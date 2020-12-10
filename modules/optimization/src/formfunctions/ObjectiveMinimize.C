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
  params.addRequiredParam<std::vector<Real>>("data_target",
                                             "Target measured value for each postprocessor.");
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
    paramError("data_target", "Computed data is inconsistent with target data.");

  Real val = 0;
  for (std::size_t i = 0; i < _data_computed.size(); ++i)
  {
    Real tmp = _data_computed[i] - _data_target[i];
    val += tmp * tmp;
  }
  val = 0.5 * val;

  return val;
}

const std::vector<Real> &
ObjectiveMinimize::getDataValueHelper(const std::string & get_param,
                                      const std::string & declare_param)
{
  if (!isParamValid(get_param) && !isParamValid(declare_param))
    mooseError("Must provide either ", get_param, " or ", declare_param, " in ", type());
  else if (isParamValid(get_param) && isParamValid(declare_param))
    paramError(declare_param, "Cannot specify both ", get_param, " and ", declare_param);
  else if (isParamValid(get_param))
    return getReporterValue<std::vector<Real>>(get_param, REPORTER_MODE_REPLICATED);
  else
    return declareValue<std::vector<Real>>(declare_param, REPORTER_MODE_REPLICATED);
}
