#ifndef CONSERVEDUNIFORMNOISE_H
#define CONSERVEDUNIFORMNOISE_H

#include "ConservedNoiseBase.h"
#include "ConservedUniformNoiseVeneer.h"

// Forward delcarations
class ConservedUniformNoise;

template <>
InputParameters
validParams<ConservedUniformNoise>()
{
  return validParams<ConservedNoiseBase>();
}

/**
 * Userobject that generates a uniformly distributed random number in the interval [-1:1]
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedUniformNoise : public ConservedUniformNoiseVeneer<ConservedNoiseBase>
{
public:
  ConservedUniformNoise(const InputParameters & parameters)
    : ConservedUniformNoiseVeneer<ConservedNoiseBase>(parameters)
  {
  }
};

#endif // CONSERVEDUNIFORMNOISE_H
