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
 * Class that calculates the HTC based on the Dittus Boelter correlation
 * It can be used for both pin and duct.
 */
class SCMHTCDittusBoelter : public SCMHTCClosureBase
{
public:
  static InputParameters validParams();

  SCMHTCDittusBoelter(const InputParameters & parameters);

  virtual Real computeNusseltNumber(const FrictionStruct & friction_info,
                                    const NusseltStruct & nusselt_info) const override;
};
