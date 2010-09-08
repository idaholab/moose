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

#include "Kernel.h"

#ifndef MMSCONVECTION_H
#define MMSCONVECTION_H

class MMSConvection;

template<>
InputParameters validParams<MMSConvection>();

class MMSConvection : public Kernel
{
public:
  
 MMSConvection(const std::string & name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

private:

  RealVectorValue velocity;

  Real _x;
  Real _y;
  Real _z;

};
#endif //MMSCONVECTION_H
