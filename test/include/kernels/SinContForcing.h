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

#ifndef SINCONTFORCING_H
#define SINCONTFORCING_H

#include "Kernel.h"

class SinContForcing;

template<>
InputParameters validParams<SinContForcing>();


class SinContForcing : public Kernel
{
public:

  SinContForcing(const std::string & name, MooseSystem & moose_system, InputParameters parameters);  

protected:
  virtual Real computeQpResidual();

  static const Real _x_center;
  static const Real _y_center;
  static const Real _area_of_influence;
  static const Real _x_min;
  static const Real _x_max;
  static const Real _y_min;
  static const Real _y_max;
};
#endif //SINCONTFORCING_H
