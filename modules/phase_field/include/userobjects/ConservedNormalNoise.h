#ifndef CONSERVEDNORMALNOISE_H
#define CONSERVEDNORMALNOISE_H

#include "ConservedNoiseBase.h"

//Forward Declarations
class ConservedNormalNoise;

template<>
InputParameters validParams<ConservedNormalNoise>();

/**
 * Userobject that generates a uniformly distributed random number in the interval
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedNormalNoise : public ConservedNoiseBase
{
public:
  ConservedNormalNoise(const std::string & name, InputParameters parameters);

protected:
  Real getQpRandom();

private:
  unsigned int _phase;
  Real _Z2;
};

#endif //CONSERVEDNORMALNOISE_H
