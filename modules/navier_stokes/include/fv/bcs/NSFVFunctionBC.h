//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvectionFunctionBC.h"
#include "NSFVBase.h"
#include "FVUtils.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

class NSFVFunctionBC : public FVMatAdvectionFunctionBC, public NSFVBase
{
public:
  static InputParameters validParams();
  NSFVFunctionBC(const InputParameters & params);

  ADReal coeffCalculator(const Elem * elem);

protected:
  /**
   * interpolation overload for the velocity
   */
  void interpolate(Moose::FV::InterpMethod m,
                   ADRealVectorValue & interp_v,
                   const ADRealVectorValue & elem_v,
                   const RealVectorValue & ghost_v);

  ADReal computeQpResidual() override;

  const Function & _pressure_exact_solution;
};

#endif
