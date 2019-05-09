//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"

class StochasticToolsTransfer;
template <>
InputParameters validParams<StochasticToolsTransfer>();

class StochasticToolsTransfer : public MultiAppTransfer
{
public:
  StochasticToolsTransfer(const InputParameters & parameters);

  virtual void initializeFromMultiapp();

  virtual void executeFromMultiapp();

  virtual void finalizeFromMultiapp();

  virtual void initializeToMultiapp();

  virtual void executeToMultiapp();

  virtual void finalizeToMultiapp();
};
