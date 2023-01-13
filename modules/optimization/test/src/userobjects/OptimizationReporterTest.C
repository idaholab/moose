//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationReporterBase.h"
#include "OptimizationReporterTest.h"
#include "libmesh/petsc_vector.h"

registerMooseObject("OptimizationTestApp", OptimizationReporterTest);

InputParameters
OptimizationReporterTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Tests the requesting of Reporter values.");
  params.addRequiredParam<std::vector<Real>>("values_to_set_parameters_to",
                                             "Testing that OptimizationReporter values can be set");
  params.addParam<std::vector<Real>>("expected_lower_bounds",
                                     "Testing that OptimizationReporter lower bounds "
                                     "can be set and used in the objective calculations");
  params.addParam<std::vector<Real>>("expected_upper_bounds",
                                     "Testing that OptimizationReporter upper bounds "
                                     "can be set and used in the objective calculations");
  params.addRequiredParam<std::vector<Real>>("values_to_set_simulation_measurements_to",
                                             "Testing that OptimizationReporter simulation data "
                                             "can be set and used in the objective calculations");
  params.addRequiredParam<Real>(
      "expected_objective_value",
      "Testing that OptimizationReporter computes the objective function.");
  return params;
}

OptimizationReporterTest::OptimizationReporterTest(const InputParameters & params)
  : GeneralUserObject(params),
    _tol(1e-6),
    _my_comm(MPI_COMM_SELF),
    _optSolverParameters(std::make_unique<libMesh::PetscVector<Number>>(_my_comm))
{
}

Real
differenceBetweenTwoVectors(std::vector<Real> & a, std::vector<Real> & b)
{
  std::transform(a.cbegin(), a.cend(), b.cbegin(), b.begin(), std::minus<>{});
  return std::abs(std::accumulate(b.begin(), b.end(), 0.0));
}

void
OptimizationReporterTest::initialSetup()
{
  if (!_fe_problem.hasUserObject("OptimizationReporter"))
    mooseError("No form function object found.");
  _optReporter = &_fe_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");
  // Set initial conditions
  _optReporter->setInitialCondition(*_optSolverParameters.get());
  std::size_t ndof = _optSolverParameters->size();
  std::vector<Real> valuesToSetOnOptRepParams(
      getParam<std::vector<Real>>("values_to_set_parameters_to"));
  if (ndof != valuesToSetOnOptRepParams.size())
    mooseError("OptimizationReporter parameter size is not the same as OptimizationReporterTest "
               "values_to_set_parameters_to size.");

  std::vector<Real> lower_bounds(ndof);
  std::vector<Real> upper_bounds(ndof);
  for (const auto & i : make_range(ndof))
  {
    lower_bounds[i] = _optReporter->getLowerBound(i);
    upper_bounds[i] = _optReporter->getUpperBound(i);
  }

  std::vector<Real> expectedLowerBounds = isParamValid("expected_lower_bounds")
                                              ? getParam<std::vector<Real>>("expected_lower_bounds")
                                              : std::vector<Real>(ndof, 0.0);
  Real diff = differenceBetweenTwoVectors(lower_bounds, expectedLowerBounds);
  if (diff > _tol)
    mooseError("Difference between OptimizationReporter lower_bounds and OptimizationReporterTest "
               "expected_lower_bounds is ",
               diff);

  std::vector<Real> expectedUpperBounds = isParamValid("expected_upper_bounds")
                                              ? getParam<std::vector<Real>>("expected_upper_bounds")
                                              : std::vector<Real>(ndof, 0.0);
  diff = differenceBetweenTwoVectors(upper_bounds, expectedUpperBounds);
  if (diff > _tol)
    mooseError("Difference between OptimizationReporter upper_bounds and OptimizationReporterTest "
               "expected_upper_bounds is ",
               diff);
}

void
OptimizationReporterTest::execute()
{
  std::vector<Real> valuesToSetOnOptRepParams(
      getParam<std::vector<Real>>("values_to_set_parameters_to"));
  size_t i = 0;
  for (auto & val : valuesToSetOnOptRepParams)
    _optSolverParameters->set(i++, val);

  _optReporter->updateParameters(*_optSolverParameters.get());
  // // Objective
  std::vector<Real> valuesToSetOnOptRepSim(
      getParam<std::vector<Real>>("values_to_set_simulation_measurements_to"));
  _optReporter->setSimulationValuesForTesting(valuesToSetOnOptRepSim);
  Real expectedObjectiveValue(getParam<Real>("expected_objective_value"));
  Real objectiveValue = _optReporter->computeObjective();
  Real diff = std::abs(expectedObjectiveValue - objectiveValue);
  if (diff > _tol)
    mooseError("OptimizationReporter objective= ",
               objectiveValue,
               " is different from that given in the input for expected_objective_value= ",
               expectedObjectiveValue);
}
