//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsoleMessageKernel.h"

registerMooseObject("MooseTestApp", ConsoleMessageKernel);

InputParameters
ConsoleMessageKernel::validParams()
{
  InputParameters params = CoefDiffusion::validParams();
  return params;
}

ConsoleMessageKernel::ConsoleMessageKernel(const InputParameters & parameters)
  : CoefDiffusion(parameters)
{
  _console << "ConsoleMessageKernel - Constructing object." << std::endl;
}

ConsoleMessageKernel::~ConsoleMessageKernel() {}

void
ConsoleMessageKernel::initialSetup()
{
  _console << "ConsoleMessageKernel::initalSetup - time = " << _t << "; t_step = " << _t_step
           << std::endl;
  constMethod();
}

void
ConsoleMessageKernel::timestepSetup()
{
  _console << "ConsoleMessageKernel::timestepSetup - time = " << _t << "; t_step = " << _t_step
           << std::endl;
}

void
ConsoleMessageKernel::constMethod() const
{
  _console << "I am writing from a const method" << std::endl;
}
