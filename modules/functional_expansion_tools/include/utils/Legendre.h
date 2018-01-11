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

#ifndef LEGENDRE_H
#define LEGENDRE_H

#include "SingleSeriesBasisInterface.h"

class Legendre final : public SingleSeriesBasisInterface
{
public:
  Legendre();
  Legendre(const std::vector<MooseEnum> & domain, const std::vector<std::size_t> & order);

  // Overrides from FunctionalBasisInterface
  virtual Real getStandardizedFunctionVolume() const override;
  virtual bool isInPhysicalBounds(const Point & point) const override;

  // Overrides from SingleSeriesBasisInterface
  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const override;
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const override;

protected:
  // Provide pure virtual methods
  // Overrides from FunctionalBasisInterface
  virtual void evaluateOrthonormal() override;
  virtual void evaluateStandard() override;

  // Overrides from SingleSeriesBasisInterface
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const override;
  virtual std::vector<Real>
  getStandardizedLocation(const std::vector<Real> & location) const override;
};

#endif // LEGENDRE_H
