//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMHTCClosureBase.h"

/**
 * Test closure that returns a negative Nusselt number.
 */
class SCMHTCNegativeTest : public SCMHTCClosureBase
{
public:
  static InputParameters validParams();

  SCMHTCNegativeTest(const InputParameters & parameters);

  Real computeNusseltNumber(const FrictionStruct & friction_info,
                            const NusseltStruct & nusselt_info) const override;
};
