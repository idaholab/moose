//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SingleSeriesBasisInterface.h"

/**
 * This class provides the algorithms and properties of the Zernike polynomial series.
 */
class Zernike : public SingleSeriesBasisInterface
{
public:
  Zernike();

  Zernike(const std::vector<MooseEnum> & domain,
          const std::vector<std::size_t> & order,
          MooseEnum expansion_type,
          MooseEnum generation_type);

  // Overrides from FunctionalBasisInterface
  virtual Real getStandardizedFunctionVolume() const override;
  virtual bool isInPhysicalBounds(const Point & point) const override;

  // Overrides from SingleSeriesBasisInterface
  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const override;
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const override;

protected:
  /**
   * Evaluates the orthonormal form of the basis functions
   */
  virtual void evaluateOrthonormal();
  /**
   * Evaluates the standard form of the basis functions
   */
  virtual void evaluateStandard();
  /**
   * Evaluates the 1/sqrt(mu) normalized form of the basis functions
   */
  virtual void evaluateSqrtMu();

  // Overrides from SingleSeriesBasisInterface
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const override;
  virtual std::vector<Real>
  getStandardizedLocation(const std::vector<Real> & location) const override;

  /**
   * Helper function used by evaluateGeneration() and evaluateExpansion(). It
   * uses the evaluated value array of the zero and positive rank terms to:
   *  1) fill out the negative rank terms
   *  2) apply the azimuthal components to all terms
   */
  void fillOutNegativeRankAndApplyAzimuthalComponent();

  /**
   * Maps the double order/rank idices to a single linear index
   */
  std::size_t simpleDoubleToSingle(std::size_t n, long m) const;

  /// Stores the recurrence evaluations for the negative rank azimuthal terms
  std::vector<Real> _negative_azimuthal_components;

  /// Stores the recurrence evaluations for the positive rank azimuthal terms
  std::vector<Real> _positive_azimuthal_components;
};
