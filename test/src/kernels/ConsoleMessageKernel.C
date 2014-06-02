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

template<>
InputParameters validParams<ConsoleMessageKernel>()
{
  InputParameters params = validParams<CoefDiffusion>();
  return params;
}

ConsoleMessageKernel::ConsoleMessageKernel(const std::string & name, InputParameters parameters) :
  CoefDiffusion(name, parameters)
{
  mooseConsole("ConsoleMessageKernel - Constructing object.");
}

ConsoleMessageKernel::~ConsoleMessageKernel()
{
}

void
ConsoleMessageKernel::initialSetup()
{
  std::ostringstream oss;
  oss << "ConsoleMessageKernel::initalSetup - time = " << _t << "; t_step = " << _t_step;
  mooseConsole(oss);
}

void
ConsoleMessageKernel::timestepSetup()
{
  std::ostringstream oss;
  oss << "ConsoleMessageKernel::timestepSetup - time = " << _t << "; t_step = " << _t_step;
  mooseConsole(oss);
}
