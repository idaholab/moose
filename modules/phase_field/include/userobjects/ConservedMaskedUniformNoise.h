#ifndef CONSERVEDMASKEDUNIFORMNOISE_H
#define CONSERVEDMASKEDUNIFORMNOISE_H

#include "ConservedMaskedNoiseBase.h"
#include "ConservedUniformNoiseVeneer.h"

// Forward delcarations
class ConservedMaskedUniformNoise;

template <>
InputParameters
validParams<ConservedMaskedUniformNoise>()
{
  return validParams<ConservedMaskedNoiseBase>();
}

/**
 * Userobject that generates a uniformly distributed random number in the interval [-1:1]
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedMaskedUniformNoise : public ConservedUniformNoiseVeneer<ConservedMaskedNoiseBase>
{
public:
  ConservedMaskedUniformNoise(const InputParameters & parameters)
    : ConservedUniformNoiseVeneer<ConservedMaskedNoiseBase>(parameters)
  {
  }
};

#endif // CONSERVEDMASKEDUNIFORMNOISE_H
