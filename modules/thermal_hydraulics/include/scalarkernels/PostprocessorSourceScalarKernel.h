//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODEKernel.h"

/**
 * Adds an arbitrary post-processor value as a source term
 */
class PostprocessorSourceScalarKernel : public ODEKernel
{
public:
  PostprocessorSourceScalarKernel(const InputParameters & params);

  virtual Real computeQpResidual() override;

protected:
  /// Post-processor to act as source
  const PostprocessorValue & _pp;

public:
  static InputParameters validParams();
};
