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

/// Base class for implementing DG cohesive zone models (CZM) for 1D,2D, and 3D
/// traction separation laws. This kernel operates only on
/// a single displacement compenent.
/// One kernel is required for each displacement component.
class ADSCZMInterfaceKernelBase : public JvarMapKernelInterface<ADSBMInterfaceBase>
{
public:
  static InputParameters validParams();
  ADSCZMInterfaceKernelBase(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Whether or not to perform shifted integration
  virtual bool perform_shifted() const override { return _shifted; }

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// the displacement component this kernel is operating on (0=x, 1=y, 2 =z)
  const unsigned int _component;

  /// number of displacement components
  const unsigned int _ndisp;

  /// Pointer to displacement variables
  std::vector<MooseVariable *> _vars;

  /// values of the traction used
  const ADMaterialProperty<RealVectorValue> & _traction_global;

  /// Applying Shifted Integration
  const bool _shifted;

  /// Whether to add the field correction term
  const bool _field_correction;
};
