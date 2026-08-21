//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedVelocitiesUserObject.h"
#include "LMWeightedGapUserObject.h"
#include "ContactFrictionUtils.h"

template <typename>
class MooseVariableFE;

/**
 * Nodal-based mortar contact user object for frictional problem
 */
class LMWeightedVelocitiesUserObject : public WeightedVelocitiesUserObject,
                                       public LMWeightedGapUserObject
{
public:
  static InputParameters validParams();

  LMWeightedVelocitiesUserObject(const InputParameters & parameters);

  struct ContactFrameData
  {
    Moose::Contact::ContactTangentialFrame material;
    Moose::Contact::ContactTangentialFrame constraint;
  };

  /// Request physical displacement increments and frames for LM friction regularization.
  void requestFrictionRegularizationData(bool elastic_slip);

  virtual void initialize() override;
  virtual void finalize() override;

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  /// Return the normalized current-minus-old tangential displacement for one mortar LM dof.
  ADRealVectorValue tangentialDisplacementIncrement(const DofObject * dof) const;

  /// Return the magnitude of the normalized tangential displacement increment.
  ADReal tangentialSlipIncrement(const DofObject * dof) const;

  /// Return the material and smoothed nodal frames used by elastic slip.
  const ContactFrameData & contactFrames(const DofObject * dof) const;

protected:
  virtual void computeQpProperties() override;
  virtual void computeQpIProperties() override;

  void buildContactFrames();

  /// The Lagrange multiplier variables representing the tangential contact pressure
  const MooseVariableFE<Real> * const _lm_variable_tangential_one;
  const MooseVariableFE<Real> * const _lm_variable_tangential_two;

  /// Integrated relative displacement increment in the nodal mortar tangent basis.
  std::unordered_map<const DofObject *, std::array<ADReal, 2>>
      _dof_to_tangential_displacement_increment;

  /// Material and constraint frames for each LM state.
  std::unordered_map<const DofObject *, ContactFrameData> _dof_to_contact_frame;

  /// Current-minus-old relative displacement at the current mortar quadrature point.
  ADRealVectorValue _qp_relative_displacement_increment;

  const VariableValue * _secondary_x_old = nullptr;
  const VariableValue * _primary_x_old = nullptr;
  const VariableValue * _secondary_y_old = nullptr;
  const VariableValue * _primary_y_old = nullptr;
  const VariableValue * _secondary_z_old = nullptr;
  const VariableValue * _primary_z_old = nullptr;

  /// Whether physical increment and material-frame data are requested.
  bool _compute_friction_regularization_data = false;

  /// Whether objective material frames are needed for stateful elastic slip.
  bool _compute_elastic_slip_data = false;
};
