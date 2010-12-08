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

#ifndef MMSFORCING_H
#define MMSFORCING_H

#include "Kernel.h"

class MMSForcing;

template<>
InputParameters validParams<MMSForcing>();

class MMSForcing : public Kernel
{
public:

  MMSForcing(const std::string & name,
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
#endif //MMSFORCING_H
