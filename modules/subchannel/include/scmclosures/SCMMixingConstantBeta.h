//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMMixingClosureBase.h"
#include "TriSubChannelMesh.h"
#include "QuadSubChannelMesh.h"

/**
 * Class that models the turbulent mixing coefficient $\beta$ as a user-defined constant.
 */
class SCMMixingConstantBeta : public SCMMixingClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingConstantBeta(const InputParameters & parameters);

  virtual Real computeMixingParameter(const unsigned int i_gap,
                                      const unsigned int iz,
                                      const bool sweep_flow) const override;

protected:
  /// Turbulent mixing parameter
  const Real _beta;
};
