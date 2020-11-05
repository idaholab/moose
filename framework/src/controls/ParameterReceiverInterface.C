//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ParameterReceiverInterface.h"
#include "ParameterReceiver.h"

// could initilaize list of multiapp references.
ParameterReceiverInterface::ParameterReceiverInterface() {}

void
ParameterReceiverInterface::transferParameters(ParameterReceiver & receiver,
                                               const std::vector<std::string> & names,
                                               const std::vector<Real> & values) const
{
  receiver.transfer(names, values);
}
