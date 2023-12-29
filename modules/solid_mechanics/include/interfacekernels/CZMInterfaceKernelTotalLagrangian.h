//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMInterfaceKernelBase.h"

/// DG cohesive zone model kernel for the Total Lagrangian formulation.
/// This kernel assummes the traction sepration law only depends from the
/// displacement jump. One kernel is required for each displacement component
class CZMInterfaceKernelTotalLagrangian : public CZMInterfaceKernelBase
{
public:
  static InputParameters validParams();
  CZMInterfaceKernelTotalLagrangian(const InputParameters & parameters);

protected:
  Real computeDResidualDDisplacement(const unsigned int & component_j,
                                     const Moose::DGJacobianType & type) const override;

  /// method computing the jacobian contribution due to rotations and area
  /// changes
  Real JacLD(const unsigned int cc, const bool neighbor) const;

  // derivative of the traction w.r.t. the interface deformation gradient
  const MaterialProperty<RankThreeTensor> & _dPK1traction_dF;
};
