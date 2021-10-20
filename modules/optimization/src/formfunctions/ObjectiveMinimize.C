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
  Real objective_value = FormFunction::computeObjective();
  if (_bound_adjustment > 0.0)
    objective_value += _bound_adjustment;

  if (!multiapp_passed)
    objective_value = std::numeric_limits<double>::max();

  return objective_value;
}

void
ObjectiveMinimize::updateParameters(const libMesh::PetscVector<Number> & x)
{
  _bound_adjustment = 0.0;
  dof_id_type n = 0;
  for (unsigned int i = 0; i < _parameters.size(); ++i)
  {
    for (auto & val : *_parameters[i])
    {
      Real value_from_tao = x(n);
      if (hasBounds() && value_from_tao < _lower_bounds[n])
      {
        mooseWarning("Tao Optimization Parameters out of bounds.  System will be solved using the "
                     "lower bound and the objective will be altered accordingly."
                     "\nTao Parameter Value = ",
                     value_from_tao,
                     ";  lower_bound = ",
                     _lower_bounds[n]);
        val = _lower_bounds[n];
        Real diff = value_from_tao - _lower_bounds[n];
        _bound_adjustment += diff * diff;
      }
      else if (hasBounds() && value_from_tao > _upper_bounds[n])
      {
        mooseWarning("Tao Optimization Parameters out of bounds.  System will be solved using the "
                     "upper bound and the objective will be altered accordingly."
                     "\nTao Parameter Value = ",
                     value_from_tao,
                     ";  upper_bound = ",
                     _upper_bounds[n]);
        val = _upper_bounds[n];
        Real diff = value_from_tao - _upper_bounds[n];
        _bound_adjustment += diff * diff;
      }
      else
      {
        val = value_from_tao;
      }
      n++;
    }
  }
}
