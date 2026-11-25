//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpringLawBase.h"

class Function;

/**
 * Hysteretic spring law: first loading follows the backbone curve of each local axis,
 * every unloading and reloading branch follows the Masing rule (the backbone scaled by two
 * from the last reversal point) until it meets the backbone again.
 */
class MasingHystereticSpring : public SpringLawBase
{
public:
  static InputParameters validParams();

  MasingHystereticSpring(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) override;

  /// Backbone curve of each local axis, a function of the x coordinate
  std::vector<const Function *> _backbones;

  /// Force per unit tributary area of the previous step, the reversal force at a reversal
  MaterialProperty<RealVectorValue> & _force_per_area;
  const MaterialProperty<RealVectorValue> & _force_per_area_old;

  /// Deformation of the previous step, per axis
  MaterialProperty<RealVectorValue> & _deformation_old_state;
  const MaterialProperty<RealVectorValue> & _deformation_old;

  /// Loading direction of the previous step per axis, stored as a real: +1, -1, or 0 before any
  /// loading (the state of each axis is a real so that the three axes share one vector property)
  MaterialProperty<RealVectorValue> & _direction;
  const MaterialProperty<RealVectorValue> & _direction_old;

  /// Whether each axis follows the backbone (1) or a Masing branch (0)
  MaterialProperty<RealVectorValue> & _on_backbone;
  const MaterialProperty<RealVectorValue> & _on_backbone_old;

  /// Side of the backbone the current branch runs on per axis: +1 above, -1 below, 0 unknown
  MaterialProperty<RealVectorValue> & _branch_side;
  const MaterialProperty<RealVectorValue> & _branch_side_old;

  /// Deformation and force at the last reversal per axis
  MaterialProperty<RealVectorValue> & _reversal_deformation;
  const MaterialProperty<RealVectorValue> & _reversal_deformation_old;
  MaterialProperty<RealVectorValue> & _reversal_force;
  const MaterialProperty<RealVectorValue> & _reversal_force_old;
};
