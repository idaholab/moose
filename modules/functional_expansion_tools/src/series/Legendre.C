//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Legendre.h"
#include <functional>

/**
 * The highest order of Legendre polynomials calculated directly instead of via the recurrence
 * relation
 */
#define MAX_DIRECT_CALCULATION_LEGENDRE 12

Legendre::Legendre() : SingleSeriesBasisInterface() {}

Legendre::Legendre(const std::vector<MooseEnum> & domain,
                   const std::vector<std::size_t> & order,
                   MooseEnum expansion_type,
                   MooseEnum generation_type)
  : SingleSeriesBasisInterface(domain, order, calculatedNumberOfTermsBasedOnOrder(order))
{
  if (expansion_type == "orthonormal")
    _evaluateExpansionWrapper = [this]() { this->evaluateOrthonormal(); };
  else if (expansion_type == "sqrt_mu")
    _evaluateExpansionWrapper = [this]() { this->evaluateSqrtMu(); };
  else if (expansion_type == "standard")
    _evaluateExpansionWrapper = [this]() { this->evaluateStandard(); };
  else
    mooseError("The specified type of normalization for expansion does not exist");

  if (generation_type == "orthonormal")
    _evaluateGenerationWrapper = [this]() { this->evaluateOrthonormal(); };
  else if (generation_type == "sqrt_mu")
    _evaluateGenerationWrapper = [this]() { this->evaluateSqrtMu(); };
  else if (generation_type == "standard")
    _evaluateGenerationWrapper = [this]() { this->evaluateStandard(); };
  else
    mooseError("The specified type of normalization for generation does not exist");
}

std::size_t
Legendre::calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const
{
  return order[0] + 1;
}

void
Legendre::checkPhysicalBounds(const std::vector<Real> & bounds) const
{
  // Each Legendre series should have a min and max bound
  if (bounds.size() != 2)
    mooseError("Legend: Invalid number of bounds specified for single series!");
}

void
Legendre::evaluateOrthonormal()
{
  std::size_t k;
  const Real & x = _standardized_location[0];
  const Real x2 = x * x;

  /*
   * Use direct formula to efficiently evaluate the polynomials for n <= 12
   *
   * The performance benefit diminishes for higher n. It is expected that the cost of the direct
   * calculation nears that of the recurrence relation in the neighborhood of n == 15, although this
   * theory is untested due to only implementing the direct calculations up to n == 12.
   *
   * If you want to calculate the higher-order Legendre Coefficients and code them in then be my
   * guest.
   */
  // clang-format off
  switch (_orders[0])
  {
    default:
    case MAX_DIRECT_CALCULATION_LEGENDRE:  /* 12 */
      save(12, ((((((676039 * x2 - 1939938) * x2 + 2078505) * x2 - 1021020) * x2 + 225225) * x2 - 18018) * x2 + 231) / 1024
               * 12.5);
      libmesh_fallthrough();

    case 11:
      save(11, (((((88179 * x2 - 230945) * x2 + 218790) * x2 - 90090) * x2 + 15015) * x2 - 693) * x / 256
               * 11.5);
      libmesh_fallthrough();

    case 10:
      save(10, (((((46189 * x2 - 109395) * x2 + 90090) * x2 - 30030) * x2 + 3465) * x2 - 63) / 256
               * 10.5);
      libmesh_fallthrough();

    case 9:
      save(9, ((((12155 * x2 - 25740) * x2 + 18018) * x2 - 4620) * x2 + 315) * x / 128
              * 9.5);
      libmesh_fallthrough();

    case 8:
      save(8, ((((6435 * x2 - 12012) * x2 + 6930) * x2 - 1260) * x2 + 35) / 128
              * 8.5);
      libmesh_fallthrough();

    case 7:
      save(7, (((429 * x2 - 693) * x2 + 315) * x2 - 35) * x / 16
              * 7.5);
      libmesh_fallthrough();

    case 6:
      save(6, (((231 * x2 - 315) * x2 + 105) * x2 - 5) / 16
              * 6.5);
      libmesh_fallthrough();

    case 5:
      save(5, ((63 * x2 - 70) * x2 + 15) * x / 8
              * 5.5);
      libmesh_fallthrough();

    case 4:
      save(4, ((35 * x2 - 30) * x2 + 3) / 8
              * 4.5);
      libmesh_fallthrough();

    case 3:
      save(3, (5 * x2 - 3) * x / 2
              * 3.5);
      libmesh_fallthrough();

    case 2:
      save(2, (3 * x2 - 1) / 2
              * 2.5);
      libmesh_fallthrough();

    case 1:
      save(1, x
              * 1.5);
      libmesh_fallthrough();

    case 0:
      save(0, 1
              * 0.5);
  }
  // clang-format on

  /*
   * Evaluate any remaining polynomials.
   *
   * The original recurrence relation is:
   *       (2 * k - 1) * x * L_(k-1) - (k - 1) * L_(k-2)
   * L_k = ---------------------------------------------
   *                        k
   *
   * However, for FXs we are using a the orthonormalized version of the polynomials, so each
   * polynomial L_k is multiplied by:
   *    (2 * k + 1)
   *    -----------      essentially:    k + 0.5
   *         2
   * Reversing this in the previous polynomials and implementing for the current polynomial results
   * in the orthonormalized recurrence:
   *       (2 * k + 1)   /                 (k - 1)             \
   * L_k = ----------- * | x * L_(k-1) - ----------- * L_(k-2) |
   *            k        \               (2 * k - 3)           /
   *
   * The options are 1) to use this form, or 2) to not apply the orthonormalization at first, and
   * then loop through all the values in a second loop and then apply the orthonormalization.
   */
  for (k = MAX_DIRECT_CALCULATION_LEGENDRE + 1; k <= _orders[0]; ++k)
    save(k, ((k + k + 1) / Real(k)) * (x * load(k - 1) - ((k - 1) / (k + k - 3.0)) * load(k - 2)));
}

void
Legendre::evaluateStandard()
{
  std::size_t k;
  const Real x = _standardized_location[0];
  const Real x2 = x * x;

  /*
   * Use direct formula to efficiently evaluate the polynomials for n <= 12
   *
   * The performance benefit diminishes for higher n. It is expected that the cost of the direct
   * calculation nears that of the recurrence relation in the neighborhood of n == 15, although this
   * theory is untested due to only implementing the direct calculations up to n == 12.
   *
   * If you want to calculate the higher-order Legendre Coefficients and
   * code them in then be my guest.
   */
  // clang-format off
  switch (_orders[0])
  {
    default:
    case MAX_DIRECT_CALCULATION_LEGENDRE:  /* 12 */
      save(12, ((((((676039 * x2 - 1939938) * x2 + 2078505) * x2 - 1021020) * x2 + 225225) * x2 - 18018) * x2 + 231) / 1024);
      libmesh_fallthrough();

    case 11:
      save(11, (((((88179 * x2 - 230945) * x2 + 218790) * x2 - 90090) * x2 + 15015) * x2 - 693) * x / 256);
      libmesh_fallthrough();

    case 10:
      save(10, (((((46189 * x2 - 109395) * x2 + 90090) * x2 - 30030) * x2 + 3465) * x2 - 63) / 256);
      libmesh_fallthrough();

    case 9:
      save(9, ((((12155 * x2 - 25740) * x2 + 18018) * x2 - 4620) * x2 + 315) * x / 128);
      libmesh_fallthrough();

    case 8:
      save(8, ((((6435 * x2 - 12012) * x2 + 6930) * x2 - 1260) * x2 + 35) / 128);
      libmesh_fallthrough();

    case 7:
      save(7, (((429 * x2 - 693) * x2 + 315) * x2 - 35) * x / 16);
      libmesh_fallthrough();

    case 6:
      save(6, (((231 * x2 - 315) * x2 + 105) * x2 - 5) / 16);
      libmesh_fallthrough();

    case 5:
      save(5, ((63 * x2 - 70) * x2 + 15) * x / 8);
      libmesh_fallthrough();

    case 4:
      save(4, ((35 * x2 - 30) * x2 + 3) / 8);
      libmesh_fallthrough();

    case 3:
      save(3, (5 * x2 - 3) * x / 2);
      libmesh_fallthrough();

    case 2:
      save(2, (3 * x2 - 1) / 2);
      libmesh_fallthrough();

    case 1:
      save(1, x);
      libmesh_fallthrough();

    case 0:
      save(0, 1);
  }
  // clang-format on

  /*
   * Evaluate any remaining polynomials.
   * The recurrence relation is:
   *       (2 * k - 1) * x * L_(k-1) - (k - 1) * L_(k-2)
   * L_k = ---------------------------------------------
   *                        k
   */
  for (k = MAX_DIRECT_CALCULATION_LEGENDRE + 1; k <= _orders[0]; ++k)
    save(k, (((2 * k - 1) * x * load(k - 1)) - ((k - 1) * load(k - 2))) / Real(k));
}

void
Legendre::evaluateSqrtMu()
{
  evaluateStandard();
  for (size_t i = 0; i < getNumberOfTerms(); ++i)
    save(i, load(i) * std::sqrt(i + 0.5));
}

const std::vector<Real> &
Legendre::getStandardizedFunctionLimits() const
{
  // Lazily instantiate the function limits array
  static const std::vector<Real> standardizedFunctionLimits = {-1, 1};

  return standardizedFunctionLimits;
}

Real
Legendre::getStandardizedFunctionVolume() const
{
  return 2.0; // Span of [-1, 1]
}

std::vector<Real>
Legendre::getStandardizedLocation(const std::vector<Real> & location) const
{
  const Real difference = location[0] - _physical_bounds[0];
  const Real span = _physical_bounds[1] - _physical_bounds[0];

  // Convert to [0, 1] (assuming that location[0] is within _physical_bounds)
  const Real ratio = difference / span;

  // Legendre space is [-1, 1]
  return {ratio * 2 - 1};
}

bool
Legendre::isInPhysicalBounds(const Point & point) const
{
  std::vector<Real> location = extractLocationFromPoint(point);

  if (location[0] < _physical_bounds[0] || _physical_bounds[1] < location[0])
    return false;
  else
    return true;
}
