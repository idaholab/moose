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

class ParameterReceiver;

class ParameterReceiverInterface
{
public:
  /**
   * Constructor
   */
  ParameterReceiverInterface();

  /**
   * This class gives access to the transfer method for controllable values
   * @param receiver - ParameterReceiver for the transfer
   * @param names - controllable object names
   * @param values - value to give to controllable objects
   */
  void transferParameters(ParameterReceiver & receiver,
                          const std::vector<std::string> & names,
                          const std::vector<Real> & values) const;
};
