/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ZERNIKE_H
#define ZERNIKE_H

#include "SingleSeriesBasisInterface.h"

class Zernike : public SingleSeriesBasisInterface
{
public:
  Zernike();
  Zernike(const std::vector<MooseEnum> & domain, const std::vector<std::size_t> & order);
  virtual ~Zernike() = default;

  // Provide pure virtual methods
  virtual bool isInPhysicalBounds(const Point & point) const override;

  // Return the standardized limits of a Zernike polynomial
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const override;
  virtual Real getStandardizedFunctionVolume() const override;

  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const override;

protected:
  // Provide pure virtual methods
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const override;
  virtual void evaluateOrthonormal() override;
  virtual void evaluateStandard() override;
  virtual std::vector<Real>
  getStandardizedLocation(const std::vector<Real> & location) const override;

  /**
   * Helper function used by evaluateOrthonormal() and evaluateStandard(). It
   * uses the evaluated value array of the zero and positive rank terms to:
   *  1) fill out the negative rank terms
   *  2) apply the azimuthal components to all terms
   */
  void fillOutNegativeRankAndApplyAzimuthalComponent();

  /// Maps the double order/rank idices to a single linear index
  std::size_t simpleDoubleToSingle(std::size_t n, long m) const;

  /// Stores the recurrence evaluations for the negative rank azimuthal terms
  std::vector<Real> _negative_azimuthal_components;

  /// Stores the recurrence evaluations for the positive rank azimuthal terms
  std::vector<Real> _positive_azimuthal_components;
};

#endif // ZERNIKE_H
