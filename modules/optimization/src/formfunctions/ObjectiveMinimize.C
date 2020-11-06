#include "ObjectiveMinimize.h"
#include "PostprocessorInterface.h"

registerMooseObject("isopodApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "data_computed",
      "List of names of postprocessors used to obtain measurement values from simulation.");
  params.addRequiredParam<std::vector<Real>>("data_target",
                                             "Target measured value for each postprocessor.");
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _data_computed(parameters.get<std::vector<PostprocessorName>>("data_computed").size()),
    _data_target(getParam<std::vector<Real>>("data_target"))
{
  if (_data_computed.size() != _data_target.size())
    mooseError("The number of values in data_target must equal the number of postprocessors in "
               "data_computed. ");

  auto pp_names = parameters.get<std::vector<PostprocessorName>>("data_computed");
  for (std::size_t i = 0; i < pp_names.size(); ++i)
    _data_computed[i] = &getPostprocessorValueByName(pp_names[i]);
}

Real
ObjectiveMinimize::computeObjective()
{
  Real val = 0;
  for (std::size_t i = 0; i < _data_computed.size(); ++i)
  {
    Real tmp = (*_data_computed[i]) - _data_target[i];
    val += tmp * tmp;
  }
  val = 0.5 * val;

  return val;
}
