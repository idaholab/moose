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

#ifndef POLYFORCING_H
#define POLYFORCING_H

#include "Kernel.h"

class PolyForcing;

template<>
InputParameters validParams<PolyForcing>();

class PolyForcing : public Kernel
{
public:

  PolyForcing(const std::string & name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
 
  Real f();
  
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

private:
  
  Real _x;
  Real _y;
  Real _z;
  
};
#endif //POLYFORCING_H
