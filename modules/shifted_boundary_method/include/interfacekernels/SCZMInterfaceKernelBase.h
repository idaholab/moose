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

/// Base class for AD and non-AD shifted cohesive zone interface kernels.
template <bool is_ad>
class SCZMInterfaceKernelBaseTempl : public JvarMapKernelInterface<SBMInterfaceBase<is_ad>>
{
public:
  static InputParameters validParams();
  SCZMInterfaceKernelBaseTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual(Moose::DGResidualType type) override;

  /**
   * Whether to apply the shifted integration corrections.
   *
   * Shifted integration is the default and intended mode for SBM simulations. It can be disabled
   * only to provide a non-shifted baseline with the same interface kernel and cohesive-zone model,
   * which isolates the effect of the shifted correction terms for verification and comparison.
   */
  bool perform_shifted() const override { return _shifted; }

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// The displacement component this kernel is operating on (0=x, 1=y, 2=z)
  const unsigned int _component;

  /// Number of displacement components
  const unsigned int _ndisp;

  /// Traction in the global frame
  const GenericMaterialProperty<RealVectorValue, is_ad> & _traction_global;

  /// True by default; false only when requesting the non-shifted verification baseline
  const bool _shifted;

  usingGenericInterfaceKernelMembers;
};

using SCZMInterfaceKernelBase = SCZMInterfaceKernelBaseTempl<false>;
using ADSCZMInterfaceKernelBase = SCZMInterfaceKernelBaseTempl<true>;
