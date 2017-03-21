#ifndef CONSERVEDMASKEDNORMALNOISE_H
#define CONSERVEDMASKEDNORMALNOISE_H

#include "ConservedMaskedNoiseBase.h"
#include "ConservedNormalNoiseVeneer.h"

// Forward delcarations
class ConservedMaskedNormalNoise;

template <>
InputParameters
validParams<ConservedMaskedNormalNoise>()
{
  return validParams<ConservedMaskedNoiseBase>();
}

/**
 * Userobject that generates a normaly distributed random number
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedMaskedNormalNoise : public ConservedNormalNoiseVeneer<ConservedMaskedNoiseBase>
{
public:
  ConservedMaskedNormalNoise(const InputParameters & parameters)
    : ConservedNormalNoiseVeneer<ConservedMaskedNoiseBase>(parameters)
  {
  }
};

#endif // CONSERVEDMASKEDNORMALNOISE_H
