//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGIntegratedBC.h"
#include "DiffusionHDGAssemblyHelper.h"

/**
 * Implements a fixed normal gradient boundary condition for use with a hybridized discretization of
 * the diffusion equation
 */
class DiffusionHDGPrescribedGradientBC : public HDGIntegratedBC, public DiffusionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionHDGPrescribedGradientBC(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onBoundary() override;

  /// Prescribed normal gradient along the boundary. The default is 0 for a natural boundary
  /// condition
  const Moose::Functor<Real> & _normal_gradient;
};
