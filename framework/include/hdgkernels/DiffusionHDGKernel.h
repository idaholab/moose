//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"
#include "DiffusionHDGAssemblyHelper.h"

#include <vector>

/**
 * Implements the diffusion equation for a hybridized discretization
 */
class DiffusionHDGKernel : public HDGKernel, public DiffusionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionHDGKernel(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onElement() override;
  virtual void onInternalSide() override;

private:
  /// optional source
  const Moose::Functor<Real> & _source;
};
