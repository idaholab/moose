//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CapabilityUtils.h"

namespace CapabilityUtils
{

Status
check(const std::string & requirement, const Registry & capabilities)
{
  Status status;
  std::string message = "Popel!";
  return {status, message};
}

} // namespace CapabilityUtils
