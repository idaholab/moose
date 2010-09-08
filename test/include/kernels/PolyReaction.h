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

#ifndef POLYREACTION_H
#define POLYREACTION_H

#include "Kernel.h"

class PolyReaction;

template<>
InputParameters validParams<PolyReaction>();

class PolyReaction : public Kernel
{
public:

  PolyReaction(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //POLYREACTION_H
