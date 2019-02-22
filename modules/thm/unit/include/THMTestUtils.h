#ifndef RELAP7TESTUTILS_H
#define RELAP7TESTUTILS_H

#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "MooseTypes.h"

// Absolute tolerance to use for calculations that should only differ by the
// accumulation of roundoff
#define ABS_TOL_ROUNDOFF 1e-12

// Minimum perturbation to be used in FD Jacobian computations
#define PERTURBATION_MIN 1e-12

Real computeFDPerturbation(const Real & value,
                           const Real & relative_perturbation = REL_PERTURBATION);

#endif // RELAP7TESTUTILS_H
