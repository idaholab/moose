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

#ifndef MMSREACTION_H
#define MMSREACTION_H

#include "Kernel.h"

class MMSReaction;

template<>
InputParameters validParams<MMSReaction>();

class MMSReaction : public Kernel
{
public:

  MMSReaction(const std::string & name, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //MMSREACTION_H
