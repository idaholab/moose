/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSERVEDNOISEINTERFACE_H
#define CONSERVEDNOISEINTERFACE_H

#include "ElementUserObject.h"

// Forward Declarations
class ConservedNoiseInterface;

/**
  * This Userobject is the base class of Userobjects that generate one
  * random number per timestep and quadrature point in a way that the integral
  * over all random numbers is zero. This can be used for a concentration fluctuation
  * kernel such as ConservedLangevinNoise, that keeps the total concenration constant.
  *
  * \see ConservedUniformNoise
  * \see ConservedNormalNoise
  */
class ConservedNoiseInterface : public ElementUserObject
{
public:
  ConservedNoiseInterface(const InputParameters & parameters);
  virtual ~ConservedNoiseInterface() {}

  virtual Real getQpValue(dof_id_type element_id, unsigned int qp) const = 0;

protected:
  virtual Real getQpRandom() = 0;

  Real _integral;
  Real _volume;
  Real _offset;

  unsigned int _qp;
};

#endif // CONSERVEDNOISEINTERFACE_H
