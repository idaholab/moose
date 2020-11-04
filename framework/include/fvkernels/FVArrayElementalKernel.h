//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernelBase.h"
#include "MooseVariableFV.h"
#include "MooseVariableInterface.h"

/// FVArrayElementalKernel is used for calculating array variable residual
/// contributions from volume integral terms of a PDE where the divergence
/// theorem is not applied (e.g.  time derivative terms, source terms, etc.).
/// As with finite element kernels, all solution values and material properties
/// must be indexed using the _qp member.  Note that all interfaces for finite
/// volume kernels are AD-based - be sure to use AD material properties and
/// other AD values to maintain good jacobian/derivative quality.
class FVArrayElementalKernel : public FVElementalKernelBase,
                               public MooseVariableInterface<RealEigenVector>
{
public:
  static InputParameters validParams();
  FVArrayElementalKernel(const InputParameters & parameters);

  /// Usually you should not override these functions - they have some
  /// tricky stuff in them that you don't want to mess up!
  ///@{
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  ///@}

protected:
  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Note that solution gradients will be zero unless you are using a
  /// higher-order reconstruction.  Material properties and other values
  /// should be initialized just like they are for fintie element kernels here -
  /// since this is a FE-like volumetric integration term.
  virtual ADRealEigenVector computeQpResidual() = 0;

  MooseVariableFV<RealEigenVector> & _var;
  const ADArrayVariableValue & _u;
};
