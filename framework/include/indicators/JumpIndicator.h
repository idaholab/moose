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

#ifndef JUMPINDICATOR_H
#define JUMPINDICATOR_H

#include "InternalSideIndicator.h"

class JumpIndicator;

template<>
InputParameters validParams<JumpIndicator>();

class JumpIndicator :
  public InternalSideIndicator
{
public:
  JumpIndicator(const InputParameters & parameters);
  virtual ~JumpIndicator(){};

protected:

  virtual Real computeQpIndicator();
};

#endif /* JUMPINDICATOR_H */
