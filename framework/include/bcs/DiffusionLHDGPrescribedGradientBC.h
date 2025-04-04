//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "DiffusionLHDGAssemblyHelper.h"

/**
 * Implements a fixed normal gradient boundary condition for use with a hybridized discretization of
 * the diffusion equation
 */
class DiffusionLHDGPrescribedGradientBC : public IntegratedBC, public DiffusionLHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionLHDGPrescribedGradientBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// Prescribed normal gradient along the boundary. The default is 0 for a natural boundary
  /// condition
  const Moose::Functor<Real> & _normal_gradient;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};
