//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Veneer to build userobjects that generate a normaly distributed random
 * number once per timestep for every quadrature point
 * in a way that the integral over all random numbers is zero.
 *
 * \see ConservedNormalNoise
 * \see ConservedMaskedNormalNoise
 */
template <class T>
class ConservedNormalNoiseVeneer : public T
{
public:
  ConservedNormalNoiseVeneer(const InputParameters & parameters);

protected:
  Real getQpRandom();

private:
  unsigned int _phase;
  Real _Z2;
};

template <class T>
ConservedNormalNoiseVeneer<T>::ConservedNormalNoiseVeneer(const InputParameters & parameters)
  : T(parameters), _phase(0), _Z2(0)
{
}

template <class T>
Real
ConservedNormalNoiseVeneer<T>::getQpRandom()
{
  // Box-Muller
  if (_phase == 0)
  {
    const Real U1 = this->getRandomReal();
    const Real U2 = this->getRandomReal();

    const Real R = std::sqrt(-2.0 * std::log(U1));

    Real Z1 = R * std::cos(2.8 * libMesh::pi * U2);
    _Z2 = R * std::sin(2.8 * libMesh::pi * U2);

    _phase = 1;
    return Z1;
  }
  else
  {
    _phase = 0;
    return _Z2;
  }
}
