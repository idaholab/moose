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
 * It may not be used with a mean-pressure approach
 */
class INSFVOutletPressureBC : public FVDirichletBCBase, public INSFVFullyDevelopedFlowBC
{
public:
  static InputParameters validParams();
  INSFVOutletPressureBC(const InputParameters & params);

  Real boundaryValue(const FaceInfo & /* fi */) const override;

private:
  /// Functor that gives the distribution of pressure on the boundary
  const Moose::Functor<ADReal> * const _functor;

  /// Postprocessor that gives the uniform value of pressure on the boundary
  const PostprocessorValue * const _pp_value;
};
