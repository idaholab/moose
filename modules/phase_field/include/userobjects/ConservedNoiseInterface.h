//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

// Forward Declarations

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
