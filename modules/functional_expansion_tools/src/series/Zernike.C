//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseUtils.h"
#include "Zernike.h"
#include <functional>

/**
 * The higherst order of Zernike polynomials calculated directly instead of via the recurrence
 * relation
 */
#define MAX_DIRECT_CALCULATION_ZERNIKE 10

Zernike::Zernike() : SingleSeriesBasisInterface() {}

Zernike::Zernike(const std::vector<MooseEnum> & domain,
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
Zernike::calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const
{
  return ((order[0] + 1) * (order[0] + 2)) / 2;
}

void
Zernike::checkPhysicalBounds(const std::vector<Real> & bounds) const
{
  /*
   * Each single series is assumed to be a function of a single coordinate, which should only have
   * two bounds.
   */
  if (bounds.size() != 3)
    mooseError("Zernike: Invalid number of bounds specified for single series!");
}

// clang-format off
void
Zernike::evaluateOrthonormal()
{
  std::size_t n;
  long j, q;
  Real H1, H2, H3;
  const Real & rho = _standardized_location[0];
  const Real rho2 = rho * rho;
  const Real rho4 = rho2 * rho2;

  if (MooseUtils::absoluteFuzzyEqual(rho, 0.0))
  {
    for (n = 0; n <= _orders[0]; n += 2)
    {
      j = simpleDoubleToSingle(n, 0);

      if ((n / 2) % 2 != 0)
        save(j, -1 * (n + 1) / M_PI);
      else
        save(j, 1 * (n + 1) / M_PI);
    }

    return;
  }

  switch (_orders[0])
  {
    default:
    case MAX_DIRECT_CALCULATION_ZERNIKE:  /* 10 */
      save(65, rho4 * rho4 * rho2
               * 22 / M_PI);
      save(64, (10 * rho2 - 9) * rho4 * rho4
               * 22 / M_PI);
      save(63, ((45 * rho2 - 72) * rho2 + 28) * rho4 * rho2
               * 22 / M_PI);
      save(62, (((120 * rho2 - 252) * rho2 + 168) * rho2- 35) * rho4
               * 22 / M_PI);
      save(61, ((((210 * rho2 - 504) * rho2 + 420) * rho2 - 140) * rho2 + 15) * rho2
               * 22 / M_PI);
      save(60, (((((252 * rho2 - 630) * rho2 + 560) * rho2 - 210) * rho2 + 30) * rho2 - 1)
               * 11 / M_PI);
      libmesh_fallthrough();

    case 9:
      save(54, rho4 * rho4 * rho
               * 20 / M_PI);
      save(53, (9 * rho2 - 8) * rho4 * rho2 * rho
               * 20 / M_PI);
      save(52, ((36 * rho2 - 56) * rho2 + 21) * rho4 * rho
               * 20 / M_PI);
      save(51, (((84 * rho2 - 168) * rho2 + 105) * rho2 - 20) * rho2 * rho
               * 20 / M_PI);
      save(50, ((((126 * rho2 - 280) * rho2 + 210) * rho2 - 60) * rho2 + 5) * rho
               * 20 / M_PI);
      libmesh_fallthrough();

    case 8:
      save(44, rho4 * rho4
               * 18 / M_PI);
      save(43, (8 * rho2 - 7) * rho4 * rho2
               * 18 / M_PI);
      save(42, ((28 * rho2 - 42) * rho2 + 15) * rho4
               * 18 / M_PI);
      save(41, (((56 * rho2 - 105) * rho2 + 60) * rho2 - 10) * rho2
               * 18 / M_PI);
      save(40, ((((70 * rho2 - 140) * rho2 + 90) * rho2 - 20) * rho2 + 1)
               * 9 / M_PI);
      libmesh_fallthrough();

    case 7:
      save(35, rho4 * rho2 * rho
               * 16 / M_PI);
      save(34, (7 * rho2 - 6) * rho4 * rho
               * 16 / M_PI);
      save(33, ((21 * rho2 - 30) * rho2 + 10) * rho2 * rho
               * 16 / M_PI);
      save(32, (((35 * rho2 - 60) * rho2 + 30) * rho2 - 4) * rho
               * 16 / M_PI);
      libmesh_fallthrough();

    case 6:
      save(27, rho4 * rho2
               * 14 / M_PI);
      save(26, (6 * rho2 - 5) * rho4
               * 14 / M_PI);
      save(25, ((15 * rho2 - 20) * rho2 + 6) * rho2
               * 14 / M_PI);
      save(24, (((20 * rho2 - 30) * rho2 + 12) * rho2 - 1)
               * 7 / M_PI);
      libmesh_fallthrough();

    case 5:
      save(20, rho4 * rho
               * 12 / M_PI);
      save(19, (5 * rho2 - 4) * rho2 * rho
               * 12 / M_PI);
      save(18, ((10 * rho2 - 12) * rho2 + 3) * rho
               * 12 / M_PI);
      libmesh_fallthrough();

    case 4:
      save(14, rho4
               * 10 / M_PI);
      save(13, (4 * rho2 - 3) * rho2
               * 10 / M_PI);
      save(12, ((6 * rho2 - 6) * rho2 + 1)
               * 5 / M_PI);
      libmesh_fallthrough();

    case 3:
      save(9, rho2 * rho
              * 8 / M_PI);
      save(8, (3 * rho2 - 2) * rho
              * 8 / M_PI);
      libmesh_fallthrough();

    case 2:
      save(5, rho2
              * 6 / M_PI);
      save(4, (2 * rho2 - 1)
              * 3 / M_PI);
      libmesh_fallthrough();

    case 1:
      save(2, rho
              * 4 / M_PI);
      libmesh_fallthrough();

    case 0:
      save(0, 1
              * 1 / M_PI);
  }

  for (n = MAX_DIRECT_CALCULATION_ZERNIKE + 1; n <= _orders[0]; ++n)
  {
    j = simpleDoubleToSingle(n, n);
    save(j, pow(rho, n)
            * (n + n + 2) / M_PI);

    j--;
    save(j, n * load(j + 1) - (n + 1) * load(j - (n + n)));

    for (q = n; q >= 4; q -= 2)
    {
      H3 = (-4 * (q - 2) * (q - 3)) / ((n + q - 2) * (n - q + 4.0));
      H2 = (H3 * (n + q) * (n - q + 2)) / (4.0 * (q - 1)) + (q - 2);
      H1 = q * (q - 1) / 2 - q * H2 + (H3 * (n + q + 2) * (n - q)) / 8.0;
      j--;
      if (q == 4)
        save(j, (H1 * load(j + 2) + (H2 + H3 / rho2) * load(j + 1))
                * 0.5);
      else
        save(j, H1 * load(j + 2) + (H2 + H3 / rho2) * load(j + 1));
    }
  }

  fillOutNegativeRankAndApplyAzimuthalComponent();
}
// clang-format on

void
Zernike::evaluateSqrtMu()
{
  evaluateStandard();
  save(0, load(0) / std::sqrt(M_PI));
  size_t j = 1;
  for (size_t n = 1; n < _orders[0] + 1; ++n)
  {
    for (size_t m = 0; m < n + 1; ++m)
    {
      if (m != 0 && n / m == 2 && n % m == 0)
        save(j, load(j) * std::sqrt((n + 1) / M_PI));
      else
        save(j, load(j) * std::sqrt((2 * n + 2) / M_PI));
      ++j;
    }
  }
}

void
Zernike::evaluateStandard()
{
  std::size_t n;
  long j, q;
  Real H1, H2, H3;
  const Real & rho = _standardized_location[0];
  const Real rho2 = rho * rho;
  const Real rho4 = rho2 * rho2;

  if (MooseUtils::absoluteFuzzyLessEqual(rho, 0))
  {
    for (n = 0; n <= _orders[0]; n += 2)
    {
      j = simpleDoubleToSingle(n, 0);

      if ((n / 2) % 2 != 0)
        save(j, -1);
      else
        save(j, 1);
    }

    return;
  }

  switch (_orders[0])
  {
    default:
    case MAX_DIRECT_CALCULATION_ZERNIKE: /* 10 */
      save(65, rho4 * rho4 * rho2);
      save(64, (10 * rho2 - 9) * rho4 * rho4);
      save(63, ((45 * rho2 - 72) * rho2 + 28) * rho4 * rho2);
      save(62, (((120 * rho2 - 252) * rho2 + 168) * rho2 - 35) * rho4);
      save(61, ((((210 * rho2 - 504) * rho2 + 420) * rho2 - 140) * rho2 + 15) * rho2);
      save(60, ((((252 * rho2 - 630) * rho2 + 560) * rho2 - 210) * rho2 + 30) * rho2 - 1);
      libmesh_fallthrough();

    case 9:
      save(54, rho4 * rho4 * rho);
      save(53, (9 * rho2 - 8) * rho4 * rho2 * rho);
      save(52, ((36 * rho2 - 56) * rho2 + 21) * rho4 * rho);
      save(51, (((84 * rho2 - 168) * rho2 + 105) * rho2 - 20) * rho2 * rho);
      save(50, ((((126 * rho2 - 280) * rho2 + 210) * rho2 - 60) * rho2 + 5) * rho);
      libmesh_fallthrough();

    case 8:
      save(44, rho4 * rho4);
      save(43, (8 * rho2 - 7) * rho4 * rho2);
      save(42, ((28 * rho2 - 42) * rho2 + 15) * rho4);
      save(41, (((56 * rho2 - 105) * rho2 + 60) * rho2 - 10) * rho2);
      save(40, (((70 * rho2 - 140) * rho2 + 90) * rho2 - 20) * rho2 + 1);
      libmesh_fallthrough();

    case 7:
      save(35, rho4 * rho2 * rho);
      save(34, (7 * rho2 - 6) * rho4 * rho);
      save(33, ((21 * rho2 - 30) * rho2 + 10) * rho2 * rho);
      save(32, (((35 * rho2 - 60) * rho2 + 30) * rho2 - 4) * rho);
      libmesh_fallthrough();

    case 6:
      save(27, rho4 * rho2);
      save(26, (6 * rho2 - 5) * rho4);
      save(25, ((15 * rho2 - 20) * rho2 + 6) * rho2);
      save(24, ((20 * rho2 - 30) * rho2 + 12) * rho2 - 1);
      libmesh_fallthrough();

    case 5:
      save(20, rho4 * rho);
      save(19, (5 * rho2 - 4) * rho2 * rho);
      save(18, ((10 * rho2 - 12) * rho2 + 3) * rho);
      libmesh_fallthrough();

    case 4:
      save(14, rho4);
      save(13, (4 * rho2 - 3) * rho2);
      save(12, (6 * rho2 - 6) * rho2 + 1);
      libmesh_fallthrough();

    case 3:
      save(9, rho2 * rho);
      save(8, (3 * rho2 - 2) * rho);
      libmesh_fallthrough();

    case 2:
      save(5, rho2);
      save(4, 2 * rho2 - 1);
      libmesh_fallthrough();

    case 1:
      save(2, rho);
      libmesh_fallthrough();

    case 0:
      save(0, 1);
  }

  for (n = MAX_DIRECT_CALCULATION_ZERNIKE + 1; n <= _orders[0]; ++n)
  {
    j = simpleDoubleToSingle(n, n);
    save(j, pow(rho, n));

    j--;
    save(j, n * load(j + 1) - (n - 1) * load(j - (n + n)));

    for (q = n; q >= 4; q -= 2)
    {
      H3 = (-4 * (q - 2) * (q - 3)) / ((n + q - 2) * (n - q + 4.0));
      H2 = (H3 * (n + q) * (n - q + 2)) / (4.0 * (q - 1)) + (q - 2);
      H1 = q * (q - 1) / 2 - q * H2 + (H3 * (n + q + 2) * (n - q)) / 8.0;
      j--;
      save(j, H1 * load(j + 2) + (H2 + H3 / rho2) * load(j + 1));
    }
  }

  fillOutNegativeRankAndApplyAzimuthalComponent();
}

void
Zernike::fillOutNegativeRankAndApplyAzimuthalComponent()
{
  std::size_t n;
  long j, m, q, a;
  const Real & phi = _standardized_location[1];

  j = 0;
  for (n = 1; n <= _orders[0]; ++n)
  {
    j += n;
    for (m = 0, q = a = n; m < q; ++m, --q, a -= 2)
    {
      save(j + m, load(j + q) * sin(a * phi));
      save(j + q, load(j + q) * cos(a * phi));
    }
  }
}

const std::vector<Real> &
Zernike::getStandardizedFunctionLimits() const
{
  // Lazily instantiate the function limits array
  static const std::vector<Real> standardizedFunctionLimits = {0, 1, -M_PI, M_PI};

  return standardizedFunctionLimits;
}

Real
Zernike::getStandardizedFunctionVolume() const
{
  return M_PI; // The area of a unit disc is pi
}

std::vector<Real>
Zernike::getStandardizedLocation(const std::vector<Real> & location) const
{
  // Get the offset corresponding to the 'x' direction
  const Real offset1 = location[0] - _physical_bounds[0];
  // Get the offset corresponding to the 'y' direction
  const Real offset2 = location[1] - _physical_bounds[1];
  // Get the user-provided radius bound
  const Real & radius = _physical_bounds[2];
  // Covert to a radis and normalize
  const Real standardizedRadius = sqrt(offset1 * offset1 + offset2 * offset2) / radius;
  // Get the angle
  const Real theta = atan2(offset2, offset1);

  return {standardizedRadius, theta};
}

bool
Zernike::isInPhysicalBounds(const Point & point) const
{
  /*
   * Because Zernike polynomials live in RZ space, the easiest approach to check
   * this is to convert the physical location into a standardized location, then
   * check against the radius and theta bounds.
   */
  const std::vector<Real> location = extractLocationFromPoint(point);
  const std::vector<Real> standardized_location = getStandardizedLocation(location);

  /*
   * The radius (standardized_location[0]) is always positive, so only check
   * against the maximum radius (1). The theta components should always be in
   * bounds.
   */
  if (standardized_location[0] > 1.0)
    return false;
  else
    return true;
}

std::size_t
Zernike::simpleDoubleToSingle(std::size_t n, long m) const
{
  return (n * (n + 2) + m) / 2;
}
