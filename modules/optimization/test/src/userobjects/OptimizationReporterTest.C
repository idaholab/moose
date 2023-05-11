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

namespace
{
Real _namespaced_tol = 1e-6;
Real
differenceBetweenTwoVectors(std::vector<Real> a, std::vector<Real> b)
{
  std::transform(a.cbegin(), a.cend(), b.cbegin(), b.begin(), std::minus<>{});
  return std::abs(std::accumulate(b.begin(), b.end(), 0.0));
}
std::string
errorCheckMsg(const std::vector<Real> & optReporterBaseBounds,
              const std::vector<Real> & optReporterTestBounds,
              std::size_t ndof)
{
  std::string out;
  if (optReporterBaseBounds.size() != ndof || optReporterTestBounds.size() != ndof)
  {
    out += "There should be one bound per parameter read from the OptimizationDataBase object. ";
    out += "\n Number of parameter values on OptimizationDataBase object: ";
    out += std::to_string(ndof);
    out += "\n Number of bounds values read from OptimizationDataBase object: ";
    out += std::to_string(optReporterBaseBounds.size());
    out += "\n Number of bounds values read from OptimizationReporterTest object: ";
    out += std::to_string(optReporterTestBounds.size());
  }
  Real diff = differenceBetweenTwoVectors(optReporterBaseBounds, optReporterTestBounds);
  if (diff > _namespaced_tol)
  {
    std::string out("\n  OptimizationDataBase object bounds:        ");
    for (const auto & val : optReporterBaseBounds)
      out += std::to_string(val) + " ";
    out += "\n  OptimizationReporterTest object bounds: ";
    for (const auto & val : optReporterTestBounds)
      out += std::to_string(val) + " ";
  }

  return out;
}
}

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
    _my_comm(MPI_COMM_SELF),
    _optSolverParameters(std::make_unique<libMesh::PetscVector<Number>>(_my_comm))
{
}

void
OptimizationReporterTest::initialSetup()
{
  if (!_fe_problem.hasUserObject("OptimizationReporter"))
    mooseError("No optimization reporter object found.");
  _optReporter = &_fe_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");
  // Set initial conditions
  _optReporter->setInitialCondition(*_optSolverParameters.get());
  std::size_t ndof = _optSolverParameters->size();
  std::vector<Real> valuesToSetOnOptRepParams(
      getParam<std::vector<Real>>("values_to_set_parameters_to"));
  if (ndof != valuesToSetOnOptRepParams.size())
    mooseError("OptimizationReporter contains ",
               ndof,
               " and OptimizationReporterTest contains ",
               valuesToSetOnOptRepParams.size(),
               " parameters.  They must be the same.");

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
  std::vector<Real> expectedUpperBounds = isParamValid("expected_upper_bounds")
                                              ? getParam<std::vector<Real>>("expected_upper_bounds")
                                              : std::vector<Real>(ndof, 0.0);

  std::string errorCheckLowerBounds(errorCheckMsg(expectedLowerBounds, lower_bounds, ndof));
  if (!errorCheckLowerBounds.empty())
    mooseError("Error in lower bounds:  ", errorCheckLowerBounds);

  std::string errorCheckUpperBounds(errorCheckMsg(expectedUpperBounds, upper_bounds, ndof));
  if (!errorCheckUpperBounds.empty())
    mooseError("Error in upper bounds:  ", errorCheckUpperBounds);
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
  if (diff > _namespaced_tol)
    mooseError("OptimizationReporter objective= ",
               objectiveValue,
               " is different from that given in the input for expected_objective_value= ",
               expectedObjectiveValue);
}
