/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
