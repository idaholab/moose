#include "ObjectiveMinimize.h"
#include "PostprocessorInterface.h"

registerMooseObject("isopodApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "data_postprocessors",
      "List of names of postprocessors used to obtain measurement values from simulation.");
  params.addRequiredParam<std::vector<Real>>("measured_data",
                                             "Target measured value for each postprocessor.");
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _pp_values(parameters.get<std::vector<PostprocessorName>>("data_postprocessors").size()),
    _measured_values(getParam<std::vector<Real>>("measured_data"))
{
  auto pp_names = parameters.get<std::vector<PostprocessorName>>("data_postprocessors");
  for (std::size_t i = 0; i < pp_names.size(); ++i)
    _pp_values[i] = &getPostprocessorValueByName(pp_names[i]);
}

Real
ObjectiveMinimize::computeObjective()
{
  Real val = 0;
  for (std::size_t i = 0; i < _pp_values.size(); ++i)
  {
    Real tmp = (*_pp_values[i]) - _measured_values[i];
    val += tmp * tmp;
  }

  return val;
}
