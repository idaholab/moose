//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Computes the effective pump body force as a functor
 */
class NSFVPumpFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  NSFVPumpFunctorMaterial(const InputParameters & parameters);

protected:
  /// Function providing the pressure head
  /// The pressure head should be provided as head (in meters) vs. flow rate (in m3/s)
  const Function * const _pressure_head_function;

  /// Rated transversal area of the pump
  const Moose::Functor<Real> & _area_rated;

  /// Rated volume of the pump
  const Moose::Functor<Real> & _volume_rated;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// The fluid speed
  const Moose::Functor<ADReal> & _speed;

  /// Rated flow rate
  const Real & _flow_rate_rated;

  /// Actual flow rate
  const PostprocessorValue & _flow_rate;

  /// Gravity
  const RealVectorValue & _gravity;

  /// Rated Rotation Speed
  const Real & _rotation_speed_rated;

  /// Actual Rotation Speed
  const Real & _rotation_speed;

  /// Boolean to determine if flow rate scaling is necessary
  const bool _flow_rate_scaling_bool;

  /// Allow negative rotation speed
  const bool _bool_negative_rotation_speed;

  /// Symmetric pressure head function in the negative direction
  const bool _bool_symmetric_negative_pressure_head;

  /// Homologous pressure head function in the negative direction (in meters) vs. flow rate (in m3/s)
  const Function * const _pressure_head_function_negative_rotation;
};
