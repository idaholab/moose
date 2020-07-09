//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/// DG kernel implementing cohesive zone models (CZM) for a 1D/2D/3D traction
/// separation laws based on the displacement jump. This kernel operates only on
/// a single displacement compenent.
/// One kernel is required for each mesh dimension.
class CZMInterfaceKernel : public InterfaceKernel
{
public:
  static InputParameters validParams();
  CZMInterfaceKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// the displacement component this kernel is operating on (0=x, 1=y, 2 =z)
  const unsigned int _component;

  /// number of displacement components
  const unsigned int _ndisp;

  /// Coupled displacement component variable IDs
  ///@{
  std::vector<unsigned int> _disp_var;
  std::vector<unsigned int> _disp_neighbor_var;
  ///@}

  // pointer to displacement variables
  std::vector<MooseVariable *> _vars;

  // values of the traction and traction derivatives used
  ///@{
  const MaterialProperty<RealVectorValue> & _traction_global;
  const MaterialProperty<RankTwoTensor> & _traction_derivatives_global;
  ///@}
};
