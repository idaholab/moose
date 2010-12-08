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

#ifndef POLYCONVECTION_H
#define POLYCONVECTION_H

class PolyConvection;

template<>
InputParameters validParams<PolyConvection>();

class PolyConvection : public Kernel
{
public:
  
 PolyConvection(const std::string & name,
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
#endif //POLYCONVECTION_H
