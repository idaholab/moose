//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ViewFactorBase.h"

// Forward Declarations
class ViewFactorRayStudy;

/**
 * Computes the view factors for planar faces in unobstructed radiative heat transfer
 */
class RayTracingViewFactor : public ViewFactorBase
{
public:
  static InputParameters validParams();

  RayTracingViewFactor(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;

protected:
  virtual void threadJoinViewFactor(const UserObject & y) override;
  virtual void finalizeViewFactor() override;

  const ViewFactorRayStudy & _ray_study;
};
