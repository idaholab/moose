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
 * This class provides the algorithms and properties of the Legendre polynomial series.
 */
class Legendre final : public SingleSeriesBasisInterface
{
public:
  Legendre();
  Legendre(const std::vector<MooseEnum> & domain,
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
};
