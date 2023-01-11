//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationReporterBase.h"

InputParameters
OptimizationReporterBase::validParams()
{
  InputParameters params = OptimizationData::validParams();
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : OptimizationData(parameters)
{
}

Real
OptimizationReporterBase::computeObjective()
{
  // This will only be executed if measurement_values are available on the main app
  for (const auto i : index_range(_measurement_values))
    _misfit_values[i] = _simulation_values[i] - _measurement_values[i];

  Real val = 0.0;
  for (auto & misfit : _misfit_values)
    val += misfit * misfit;

  return val * 0.5;
}

void
OptimizationReporterBase::setMisfitToSimulatedValues()
{
  _misfit_values = _simulation_values;
}

// function only used for test objects
void
OptimizationReporterBase::setSimulationValuesForTesting(std::vector<Real> & data)
{
  _simulation_values.clear();
  _simulation_values = data;
}

void
OptimizationReporterBase::computeGradient(libMesh::PetscVector<Number> &) const
{
  mooseError(type(), " does not have an implemented gradient computation.");
}

Real OptimizationReporterBase::getUpperBound(dof_id_type) const
{
  mooseError(type(), " does not have an implemented upper bound.");
  return 0;
}

Real OptimizationReporterBase::getLowerBound(dof_id_type) const
{
  mooseError(type(), " does not have an implemented lower bound.");
  return 0;
}
