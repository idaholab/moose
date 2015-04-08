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
