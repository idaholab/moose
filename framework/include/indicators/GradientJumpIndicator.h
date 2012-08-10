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

#ifndef GRADIENTJUMPINDICATOR_H
#define GRADIENTJUMPINDICATOR_H

#include "JumpIndicator.h"

class GradientJumpIndicator;

template<>
InputParameters validParams<GradientJumpIndicator>();

class GradientJumpIndicator :
  public JumpIndicator
{
public:
  GradientJumpIndicator(const std::string & name, InputParameters parameters);
  virtual ~GradientJumpIndicator(){};

protected:

  virtual Real computeQpIndicator();
};

#endif /* GRADIENTJUMPINDICATOR_H */
