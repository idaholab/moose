//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RepeatableRayStudy.h"

class SingleTraceLineSourceTest : public RepeatableRayStudy
{
public:
  SingleTraceLineSourceTest(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void execute() override;

protected:
  /// the name of the tag that stores the residuals calculated by ray kernels
  const TagName & _residual_tag_name;
  /// whether or not the raytracing study has moved the rays on the current time step
  bool _has_traced;
};
