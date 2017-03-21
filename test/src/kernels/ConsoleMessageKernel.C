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

#include "ConsoleMessageKernel.h"

template <>
InputParameters
validParams<ConsoleMessageKernel>()
{
  InputParameters params = validParams<CoefDiffusion>();
  return params;
}

ConsoleMessageKernel::ConsoleMessageKernel(const InputParameters & parameters)
  : CoefDiffusion(parameters)
{
  _console << "ConsoleMessageKernel - Constructing object.\n";
}

ConsoleMessageKernel::~ConsoleMessageKernel() {}

void
ConsoleMessageKernel::initialSetup()
{
  _console << "ConsoleMessageKernel::initalSetup - time = " << _t << "; t_step = " << _t_step
           << '\n';
  constMethod();
}

void
ConsoleMessageKernel::timestepSetup()
{
  _console << "ConsoleMessageKernel::timestepSetup - time = " << _t << "; t_step = " << _t_step
           << '\n';
}

void
ConsoleMessageKernel::constMethod() const
{
  _console << "I am writing from a const method\n";
}
