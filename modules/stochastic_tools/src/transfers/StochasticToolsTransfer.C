//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "StochasticToolsTransfer.h"

template <>
InputParameters
validParams<StochasticToolsTransfer>()
{
  return validParams<MultiAppTransfer>();
}

StochasticToolsTransfer::StochasticToolsTransfer(const InputParameters & parameters) :
  MultiAppTransfer(parameters)
{
}

void
StochasticToolsTransfer::initializeFromMultiapp()
{
}

void
StochasticToolsTransfer::executeFromMultiapp()
{
}

void
StochasticToolsTransfer::finalizeFromMultiapp()
{
}

void
StochasticToolsTransfer::initializeToMultiapp()
{
}

void
StochasticToolsTransfer::executeToMultiapp()
{
}

void
StochasticToolsTransfer::finalizeToMultiapp()
{
}
