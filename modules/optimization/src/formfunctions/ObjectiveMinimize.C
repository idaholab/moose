#include "ObjectiveMinimize.h"

registerMooseObject("isopodApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters) : FormFunction(parameters)
{
}

Real
ObjectiveMinimize::computeAndCheckObjective(bool multiapp_passed)
{
  Real objective_value = std::numeric_limits<double>::max();

  if (multiapp_passed)
  {
    for (auto i = 0; i < _parameters.size(); ++i)
    {
      for (auto & val : *_parameters[i])
      {
        if (val > _lower_bounds[i] && val < _upper_bounds[i])

        {
          objective_value = FormFunction::computeObjective();
        }
        else
        {
          objective_value = std::numeric_limits<double>::max();
          mooseWarning("Optimization Parameters out of bounds.   Parameter Value = ",
                       val,
                       ";  lower_bound = ",
                       _lower_bounds[i],
                       ";  upper_bound = ",
                       _upper_bounds[i]);
          return objective_value;
        }
      }
    }
  }

  return objective_value;
}
