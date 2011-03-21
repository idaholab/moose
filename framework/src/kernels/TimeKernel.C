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

#include "TimeKernel.h"

template<>
InputParameters validParams<TimeKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

TimeKernel::TimeKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}
