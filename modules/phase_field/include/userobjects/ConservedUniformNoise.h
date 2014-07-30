#ifndef CONSERVEDUNIFORMNOISE_H
#define CONSERVEDUNIFORMNOISE_H

#include "ConservedNoiseBase.h"

//Forward Declarations
class ConservedUniformNoise;

template<>
InputParameters validParams<ConservedUniformNoise>();

/**
 * Userobject that generates a uniformly distributed random number in the interval
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedUniformNoise : public ConservedNoiseBase
{
public:
  ConservedUniformNoise(const std::string & name, InputParameters parameters);

protected:
  Real getQpRandom();
};

#endif //CONSERVEDUNIFORMNOISE_H
