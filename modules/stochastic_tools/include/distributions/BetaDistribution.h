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

#ifndef BETADISTRIBUTION_H
#define BETADISTRIBUTION_H

#include "Distribution.h"
#include "distribution_1D.h"

class BetaDistribution;

template <>
InputParameters validParams<BetaDistribution>();
/**
 * A class used to generate beta distribution from RAVEN crow
 */
class BetaDistribution : public Distribution, BasicBetaDistribution
{
public:
  BetaDistribution(const InputParameters & parameters);
  virtual ~BetaDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
};

#endif /* BETADISTRIBUTION_H */
