//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvection.h"
#include "NSFVBase.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

class NSFVKernel : public FVMatAdvection, protected NSFVBase
{
public:
  static InputParameters validParams();
  NSFVKernel(const InputParameters & params);

protected:
  /**
   * interpolation overload for the velocity
   */
  void interpolate(Moose::FV::InterpMethod m,
                   ADRealVectorValue & interp_v,
                   const ADRealVectorValue & elem_v,
                   const ADRealVectorValue & neighbor_v);

  ADReal computeQpResidual() override;

  void residualSetup() override { clearRCCoeffs(); }
  void jacobianSetup() override { clearRCCoeffs(); }
};

#endif
