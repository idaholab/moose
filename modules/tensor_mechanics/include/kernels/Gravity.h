/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAVITY_H
#define GRAVITY_H

#include "BodyForce.h"

//Forward Declarations
class Gravity;

template<>
InputParameters validParams<Gravity>();

/**
 * Gravity computes the body force (force/volume) given the acceleration of gravity (value) and the density
 */
class Gravity : public BodyForce
{
public:

  Gravity(const std::string & name, InputParameters parameters);

  virtual ~Gravity() {}

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<Real> & _density;

};

#endif
