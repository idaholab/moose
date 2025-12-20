//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseObject.h"
#include "InputParameters.h"

class FEProblemBase;
class FVInterpolationMethod;

/**
 * Helper interface for objects that need access to FVInterpolationMethod instances.
 */
class FVInterpolationMethodInterface
{
public:
  FVInterpolationMethodInterface(const MooseObject * moose_object);

  static InputParameters validParams();

  const FVInterpolationMethod &
  getFVInterpolationMethod(const InterpolationMethodName & name) const;

  bool hasFVInterpolationMethod(const InterpolationMethodName & name) const;

protected:
  const InputParameters & _fvim_params;
  FEProblemBase & _fvim_feproblem;
  const THREAD_ID _fvim_tid;
};
