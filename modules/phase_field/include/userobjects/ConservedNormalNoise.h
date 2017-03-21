#ifndef CONSERVEDNORMALNOISE_H
#define CONSERVEDNORMALNOISE_H

#include "ConservedNoiseBase.h"
#include "ConservedNormalNoiseVeneer.h"

// Forward delcarations
class ConservedNormalNoise;

template <>
InputParameters
validParams<ConservedNormalNoise>()
{
  return validParams<ConservedNoiseBase>();
}

/**
 * Userobject that generates a normaly distributed random number
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedNormalNoise : public ConservedNormalNoiseVeneer<ConservedNoiseBase>
{
public:
  ConservedNormalNoise(const InputParameters & parameters)
    : ConservedNormalNoiseVeneer<ConservedNoiseBase>(parameters)
  {
  }
};

#endif // CONSERVEDNORMALNOISE_H
