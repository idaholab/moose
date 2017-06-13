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

#ifndef TRIANGULARDISTRIBUTION_H
#define TRIANGULARDISTRIBUTION_H

#include "Distribution.h"
#include "distribution_1D.h"

class UniformDistribution;

template <>
InputParameters validParams<TriangularDistribution>();
/**
 * A class used to generate a triangular distribution from contrib RAVEN crow
 */
class TriangularDistribution : public Distribution, BasicTriangularDistribution
{
public:
  TriangularDistribution(const InputParameters & parameters);
  virtual ~TriangularDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
};

#endif /* TRIANGULARDISTRIBUTION_H */
