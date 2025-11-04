//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

/**
 * Enum for Convergence iteration type, which can have new values registered.
 */
class ConvergenceIterationTypeEnum : public MooseEnum
{
public:
  ConvergenceIterationTypeEnum() : MooseEnum("") {}

  /// Add a new Convergence iteration type
  const MooseEnumItem & addConvergenceIterationType(const std::string & raw_name)
  {
    return addEnumerationName(raw_name);
  }
};
