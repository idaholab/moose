//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Implements a source/sink term for this object's variable/advected-quantity proportional to the
 * divergence of the mesh velocity
 */
class INSFVMeshAdvection : public FVElementalKernel
{
public:
  static InputParameters validParams();

  INSFVMeshAdvection(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /**
   * @returns the coefficient multiplying the advected quantity
   */
  ADReal advQuantCoeff(const Moose::ElemArg & elem_arg, const Moose::StateArg & state) const;

  /// The density
  const Moose::Functor<ADReal> & _rho;
  /// x-displacement
  const Moose::Functor<ADReal> & _disp_x;
  /// y-displacement
  const Moose::Functor<ADReal> & _disp_y;
  /// z-displacement
  const Moose::Functor<ADReal> & _disp_z;
  /// The advected quantity
  const Moose::FunctorBase<ADReal> & _adv_quant;
};
