//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowSinglePhaseBase.h"

/**
 * Action for simulation involving a single phase fully saturated fluid.
 */
class PorousFlowFullySaturated : public PorousFlowSinglePhaseBase
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturated(const InputParameters & params);

protected:
  virtual void addKernels() override;
  virtual void addMaterialDependencies() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  // Whether to multiply the fluid kernels by the fluid density
  const bool _multiply_by_density;
};
