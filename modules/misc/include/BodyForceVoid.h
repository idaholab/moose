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

#ifndef BODYFORCEVOID_H
#define BODYFORCEVOID_H

#include "Kernel.h"

//Forward Declarations
class BodyForceVoid;
class Function;

template<>
InputParameters validParams<BodyForceVoid>();

class BodyForceVoid : public Kernel
{
public:

  BodyForceVoid(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  Real _value;
  MooseArray<Real> & _c;
  const bool _has_function;
  Function * const _function;
};

#endif
