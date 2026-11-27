//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

/**
 * Evaluate the polynomial sum_k c_k x^k with Horner's method.
 * Device functions must be inlineable, so this is defined in the header (Exercise 2, snippet E),
 * and the running value is a local variable rather than an object member (snippet C).
 */
KOKKOS_INLINE_FUNCTION Real
evaluatePolynomial(const Moose::Kokkos::Array<Real> & coefficients, const Real x)
{
  Real value = 0;
  for (unsigned int k = coefficients.size(); k > 0; --k)
    value = value * x + coefficients[k - 1];
  return value;
}
