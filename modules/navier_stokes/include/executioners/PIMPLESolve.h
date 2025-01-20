//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "RhieChowMassFlux.h"
#include "LinearAssemblySegregatedSolve.h"

/**
 * PIMPLE-based (PISO + SIMPLE) for transient solution object with
 * linear FV system assembly. A detailed discussion of the algorithm
 * is available in
 * @book{
 *   greenshieldsweller2022,
 *   title     = "Notes on Computational Fluid Dynamics: General Principles",
 *   author    = "Greenshields, Christopher and Weller, Henry",
 *   year      = 2022,
 *   publisher = "CFD Direct Ltd",
 *   address   = "Reading, UK"
 * }
 * This will be the basis for the SIMPLE algorithm as well, we just
 * set the PISO iterations to 0.
 */
class PIMPLESolve : public LinearAssemblySegregatedSolve
{
public:
  PIMPLESolve(Executioner & ex);

  static InputParameters validParams();

protected:
  virtual std::pair<unsigned int, Real>
  correctVelocity(const bool subtract_updated_pressure,
                  const bool recompute_face_mass_flux,
                  const SolverParams & solver_params) override;

  /// Number of H(u) and u iterations with fixed face flux.
  const unsigned int _num_piso_iterations;
};
