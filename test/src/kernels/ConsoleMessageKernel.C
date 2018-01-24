//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
