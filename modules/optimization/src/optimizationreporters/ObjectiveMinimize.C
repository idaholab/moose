//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ObjectiveMinimize.h"
#include "libmesh/int_range.h"

registerMooseObject("OptimizationApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = OptimizationReporter::validParams();
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters)
  : OptimizationReporter(parameters)
{
}

Real
ObjectiveMinimize::computeAndCheckObjective(bool multiapp_passed)
{
  Real objective_value = OptimizationReporter::computeObjective();
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
  for (const auto i : index_range(_parameters))
  {
    for (auto & val : *_parameters[i])
    {
      Real value_from_tao = x(n);
      if (hasBounds() && value_from_tao < _lower_bounds[n])
      {
        mooseInfo("Tao Optimization Parameters out of bounds.  System will be solved using the "
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
        mooseInfo("Tao Optimization Parameters out of bounds.  System will be solved using the "
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
        val = value_from_tao;

      ++n;
    }
  }
}
