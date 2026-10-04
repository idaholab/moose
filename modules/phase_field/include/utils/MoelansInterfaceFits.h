//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

class InputParameters;

/**
 * Polynomial fits for the interface relations of the multi-order-parameter model of
 * Moelans et al., Phys. Rev. B 78, 024113 (2008):
 *   sigma = g(gamma) sqrt(kappa m),  l = sqrt(kappa / (m f0_interf(gamma)))
 * g(gamma) and f0_interf(gamma) are only known numerically. The fits give 1/gamma as a
 * polynomial in g^2 and f0_interf as a polynomial in 1/gamma.
 */
namespace MoelansInterfaceFits
{
/// Fits selectable through the "interface_fit" parameter
enum class Fit
{
  /// N. Moelans, Mater. Des. 217, 110592 (2022), supplementary material Sec. I
  MOELANS2022,
  /// Earlier fit from N. Moelans, parameters.m (2009)
  MOELANS2009
};

/// Parameters for objects using these fits
InputParameters validParams();

/// 1/gamma as a function of g^2
Real inverseGamma(Real g2, Fit fit);

/// f0_interf as a function of 1/gamma
Real f0Interf(Real inverse_gamma, Fit fit);

/// Whether g^2 lies within the range the fit was made for
bool inRange(Real g2, Fit fit);
}
