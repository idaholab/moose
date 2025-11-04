//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODEKernel.h"

class PostprocessorSinkScalarKernel : public ODEKernel
{
public:
  static InputParameters validParams();

  PostprocessorSinkScalarKernel(const InputParameters & parameters);

protected:
  Real computeQpResidual() final;

  const PostprocessorValue & _pp_value;
};
