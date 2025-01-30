//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Convergence.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "MooseUtils.h"

InputParameters
Convergence::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();
  params += PostprocessorInterface::validParams();
  params += PerfGraphInterface::validParams();
  params += TransientInterface::validParams();

  params.addParam<bool>(
      "verbose",
      false,
      "Enable printing of additional information, including convergence and divergence reasons.");

  params.registerBase("Convergence");

  return params;
}

Convergence::Convergence(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    PostprocessorInterface(this),
    PerfGraphInterface(this),
    TransientInterface(this),
    _perfid_check_convergence(registerTimedSection("checkConvergence", 5, "Checking Convergence")),
    _tid(getParam<THREAD_ID>("_tid")),
    _verbose(getParam<bool>("verbose") ? true : getMooseApp().getExecutioner()->verbose())
{
}

void
Convergence::verboseOutput(std::ostringstream & oss)
{
  const auto str = oss.str();

  if (str.length() == 0)
    return;

  if (verbose())
    _console << name() << ": " << MooseUtils::trim(str) << std::endl;
}
