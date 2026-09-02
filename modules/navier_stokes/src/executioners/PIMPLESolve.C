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

using namespace libMesh;

InputParameters
PIMPLESolve::validParams()
{
  InputParameters params = LinearAssemblySegregatedSolve::validParams();
  params.addParam<unsigned int>(
      "num_piso_iterations",
      0,
      "The number of PISO iterations without recomputing the momentum matrix.");

  return params;
}

PIMPLESolve::PIMPLESolve(Executioner & ex)
  : LinearAssemblySegregatedSolve(ex),
    _num_piso_iterations(getParam<unsigned int>("num_piso_iterations"))
{
}

std::pair<unsigned int, Real>
PIMPLESolve::correctVelocity(const bool /*subtract_updated_pressure*/,
                             const bool /*recompute_face_mass_flux*/,
                             const SolverParams & solver_params)
{
  std::pair<unsigned int, Real> residual;
  unsigned int piso_iteration_counter = 0;
  const bool reconstructed = _rc_uo && _rc_uo->usingReconstructedPressureGradientMethod();
  while (piso_iteration_counter <= _num_piso_iterations)
  {
    const bool first_piso_corrector = piso_iteration_counter == 0;
    const bool last_piso_corrector = piso_iteration_counter == _num_piso_iterations;

    // Reconstructed candidates always need the face flux produced by their own pressure
    // solve, so every corrector must recompute it. Ordinary (non-reconstructed) PIMPLE only
    // needs the final corrector's flux, which feeds the advection terms for the next outer
    // iteration/time step.
    const bool recompute_flux = reconstructed || last_piso_corrector;

    residual = LinearAssemblySegregatedSolve::correctVelocity(
        first_piso_corrector, recompute_flux, solver_params);

    // After each PISO corrector except the last, refresh the lagged velocity gradient from the
    // newly corrected velocity field before starting the next corrector.
    if (!last_piso_corrector && reconstructed)
      _rc_uo->captureLaggedVelocityGradient();

    piso_iteration_counter++;
  }

  return residual;
}
