//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIMPLESolve.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"
#include "LinearSystem.h"

#include <limits>

using namespace libMesh;

InputParameters
PIMPLESolve::validParams()
{
  InputParameters params = LinearAssemblySegregatedSolve::validParams();
  params.addParam<unsigned int>(
      "num_piso_iterations",
      0,
      "The maximum number of additional PISO pressure-correction stages without recomputing the "
      "momentum matrix.");
  params.addParam<Real>(
      "piso_absolute_tolerance",
      -1.0,
      "Absolute residual tolerance for terminating the inner PISO loop early. If this is <= 0, "
      "pressure_absolute_tolerance is used.");
  params.addRangeCheckedParam<Real>(
      "piso_relative_tolerance",
      0.0,
      "0.0<=piso_relative_tolerance & piso_relative_tolerance<=1.0",
      "Relative residual reduction target for terminating the inner PISO loop early, measured "
      "against the first PISO-stage pressure residual. Set to 0 to disable the relative check.");

  return params;
}

PIMPLESolve::PIMPLESolve(Executioner & ex)
  : LinearAssemblySegregatedSolve(ex),
    _num_piso_iterations(getParam<unsigned int>("num_piso_iterations")),
    _piso_absolute_tolerance(getParam<Real>("piso_absolute_tolerance")),
    _piso_relative_tolerance(getParam<Real>("piso_relative_tolerance"))
{
}

Real
PIMPLESolve::pisoAbsoluteTolerance() const
{
  return _piso_absolute_tolerance > 0.0 ? _piso_absolute_tolerance : _pressure_absolute_tolerance;
}

bool
PIMPLESolve::shouldContinuePISOIterations(const unsigned int piso_iteration_counter,
                                          const Real stage_residual,
                                          const Real first_stage_residual) const
{
  if (piso_iteration_counter >= _num_piso_iterations)
    return false;

  if (stage_residual <= pisoAbsoluteTolerance())
    return false;

  if (_piso_relative_tolerance > 0.0 &&
      first_stage_residual > std::numeric_limits<Real>::epsilon() &&
      stage_residual <= _piso_relative_tolerance * first_stage_residual)
    return false;

  return true;
}

std::pair<unsigned int, Real>
PIMPLESolve::correctVelocity(const bool /*subtract_updated_pressure*/,
                             const bool /*recompute_face_mass_flux*/,
                             const SolverParams & solver_params)
{
  std::pair<unsigned int, Real> residual;
  Real first_stage_residual = std::numeric_limits<Real>::quiet_NaN();
  unsigned int piso_iteration_counter = 0;
  while (true)
  {
    residual = LinearAssemblySegregatedSolve::correctVelocity(
        piso_iteration_counter == 0, true, solver_params);
    if (piso_iteration_counter == 0)
      first_stage_residual = residual.second;
    if (!shouldContinuePISOIterations(
            piso_iteration_counter, residual.second, first_stage_residual))
      break;
    piso_iteration_counter++;
  }

  return residual;
}
