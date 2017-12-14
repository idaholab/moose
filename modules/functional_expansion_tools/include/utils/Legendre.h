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
  virtual ~Legendre() = default;

  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const final;

  // Return the standardized limits of a Legendre polynomial
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const final;
  virtual Real getStandardizedFunctionVolume() const final;

  // Provide pure virtual methods
  virtual bool isInPhysicalBounds(const Point & point) const final;

protected:
  // Provide pure virtual methods
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const final;
  virtual void evaluateOrthonormal() final;
  virtual void evaluateStandard() final;
  virtual std::vector<Real> getStandardizedLocation(const std::vector<Real> & location) const final;
};

#endif // LEGENDRE_H
