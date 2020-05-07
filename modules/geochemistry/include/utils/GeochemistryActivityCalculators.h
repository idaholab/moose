//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MooseTypes.h"

/**
 * Calculators to compute activity coefficients
 */
namespace GeochemistryActivityCalculators
{
/// log10(activity coefficient) according to the Debye-Huckel B-dot model
Real log10ActCoeffDHBdot(
    Real charge, Real ion_size, Real sqrt_ionic_strength, Real A, Real B, Real Bdot);

/// log10(activity coefficient) for neutral species according to the Debye-Huckel B-dot model
Real log10ActCoeffDHBdotNeutral(Real ionic_strength, Real a, Real b, Real c, Real d);

/// ln(activity of water) according to the Debye-Huckel B-dot model
Real lnActivityDHBdotWater(
    Real stoichiometric_ionic_strength, Real A, Real atilde, Real btilde, Real ctilde, Real dtilde);

/// log10(activity coefficient) alternative expression that is sometimes used in conjunction with the Debye-Huckel B-dot model
Real log10ActCoeffDHBdotAlternative(Real ionic_strength, Real Bdot);

/// log10(activity coefficient) according to the Davies model
Real log10ActCoeffDavies(Real charge, Real sqrt_ionic_strength, Real A);

} // namespace GeochemistryActivityCalculators
