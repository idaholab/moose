/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAVITYTM_H
#define GRAVITYTM_H

#include "BodyForce.h"

//Forward Declarations
class GravityTM;

template<>
InputParameters validParams<GravityTM>();

/**
 * GravityTM computes the body force (force/volume) given the acceleration of gravity (value) and the density
 */
class GravityTM : public BodyForce
{
public:

  GravityTM(const std::string & name, InputParameters parameters);

  virtual ~GravityTM() {}

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<Real> & _density;
};

#endif //GRAVITYTM_H
