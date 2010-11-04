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

#ifndef TEJUMPFFN_H
#define TEJUMPFFN_H

#include "Kernel.h"

class TEJumpFFN;

template<>
InputParameters validParams<TEJumpFFN>();

class TEJumpFFN : public Kernel
{
public:

  TEJumpFFN(const std::string & name,
            MooseSystem &sys,
            InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  Real _t_jump;
  Real _slope;
};

#endif //TEJUMPFFN_H
