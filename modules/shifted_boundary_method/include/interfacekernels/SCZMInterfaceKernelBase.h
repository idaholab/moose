//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMInterfaceBase.h"
#include "JvarMapInterface.h"

/// Non-AD base class for implementing DG shifted cohesive zone models (SCZM) for 1D, 2D, and 3D
/// traction-separation laws. This is the hand-coded-Jacobian counterpart of
/// ADSCZMInterfaceKernelBase. The kernel operates only on a single displacement component; one
/// kernel is required for each displacement component.
class SCZMInterfaceKernelBase : public JvarMapKernelInterface<SBMInterfaceBase>
{
public:
  static InputParameters validParams();
  SCZMInterfaceKernelBase(const InputParameters & parameters);

protected:
  Real computeQpResidual(Moose::DGResidualType type) override;
  Real computeQpJacobian(Moose::DGJacobianType type) override;
  Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  /// Whether or not to perform shifted integration
  bool perform_shifted() const override { return _shifted; }

  /// Method computing the derivative of residual[_component] w.r.t displacement[component_j]
  virtual Real computeDResidualDDisplacement(const unsigned int & component_j,
                                             const Moose::DGJacobianType & type) const = 0;

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// The displacement component this kernel is operating on (0=x, 1=y, 2=z)
  const unsigned int _component;

  /// Number of displacement components
  const unsigned int _ndisp;

  /// Coupled displacement component variable IDs
  std::vector<unsigned int> _disp_var;

  /// Pointer to displacement variables
  std::vector<MooseVariable *> _vars;

  /// Values of the traction and its derivative w.r.t. the displacement jump
  ///@{
  const MaterialProperty<RealVectorValue> & _traction_global;
  const MaterialProperty<RankTwoTensor> & _dtraction_djump_global;
  ///@}

  /// Applying shifted integration
  const bool _shifted;
};
