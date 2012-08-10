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

#include "JumpIndicator.h"

template<>
InputParameters validParams<JumpIndicator>()
{
  InputParameters params = validParams<InternalSideIndicator>();
  return params;
}


JumpIndicator::JumpIndicator(const std::string & name, InputParameters parameters) :
    InternalSideIndicator(name, parameters)
{
}


Real
JumpIndicator::computeQpIndicator()
{
  return 0;
}

