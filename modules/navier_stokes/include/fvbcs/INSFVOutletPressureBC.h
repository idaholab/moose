//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFunctionDirichletBC.h"
#include "INSFVFullyDevelopedFlowBC.h"

/**
 * A class for setting the value of the pressure at an outlet of the system.
 * It may not be used with a mean/pinned-pressure approach
 */
class INSFVOutletPressureBC : public FVDirichletBCBase, public INSFVFullyDevelopedFlowBC
{
public:
  static InputParameters validParams();
  INSFVOutletPressureBC(const InputParameters & params);

  ADReal boundaryValue(const FaceInfo & /* fi */) const override;

private:
  /// AD Functor that gives the distribution of pressure on the boundary
  const Moose::Functor<ADReal> * const _functor;

  /// Regular function that gives the distribution of pressure on the boundary
  const Function * const _function;

  /// Postprocessor that gives the uniform value of pressure on the boundary
  const PostprocessorValue * const _pp_value;
};
