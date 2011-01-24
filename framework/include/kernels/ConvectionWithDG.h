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

#ifndef CONVECTIONWITHDG_H
#define CONVECTIONWITHDG_H

#include "Kernel.h"

// Forward Declaration
class ConvectionWithDG;


template<>
InputParameters validParams<ConvectionWithDG>();

class ConvectionWithDG : public Kernel
{
public:

  ConvectionWithDG(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
  
};
#endif // CONVECTIONWITHDG_H
