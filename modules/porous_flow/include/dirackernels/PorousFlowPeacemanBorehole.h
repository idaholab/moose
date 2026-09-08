//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLineSink.h"

class SinglePhaseFluidProperties;

namespace libMesh
{
class System;
}

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class PorousFlowPeacemanBorehole : public PorousFlowLineSink
{
public:
  /**
   * Creates a new PorousFlowPeacemanBorehole
   * This reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  static InputParameters validParams();

  PorousFlowPeacemanBorehole(const InputParameters & parameters);

  void virtual initialSetup() override;
  void virtual residualSetup() override;
  void virtual jacobianSetup() override;

protected:
  /**
   * If positive then the borehole acts as a sink (producion well) for porepressure > borehole
   * pressure, and does nothing otherwise
   * If negative then the borehole acts as a source (injection well) for porepressure < borehole
   * pressure, and does nothing otherwise
   * The flow rate to/from the borehole is multiplied by |character|, so usually character = +/- 1
   */
  const Function & _character;

  /// Bottomhole pressure of borehole
  const Function & _p_bot;

  /// Unit weight of fluid in borehole (for calculating bottomhole pressure at each Dirac Point).
  /// Not used if _use_density_from_temperature is true.
  const RealVectorValue _unit_weight;

  /// Whether the wellbore pressure profile is built from a temperature-dependent fluid density
  /// (true if the 'unit_weight_fp' parameter was supplied) rather than from the constant _unit_weight
  const bool _use_density_from_temperature;

  /// Fluid properties used to evaluate the in-well fluid density.  nullptr unless
  /// _use_density_from_temperature
  const SinglePhaseFluidProperties * const _fp;

  /// The libMesh system holding unit_weight_temperature.  nullptr unless
  /// _use_density_from_temperature
  libMesh::System * const _temperature_system;

  /// Variable number of unit_weight_temperature within _temperature_system
  const unsigned int _temperature_var_number;

  /// Gravitational acceleration (in the units used elsewhere in the input file), pointing
  /// downwards.  Only used if _use_density_from_temperature
  const RealVectorValue _gravity;

  /// Fixed pressure (Pa) at which the in-well fluid density is evaluated.  Only used if
  /// _use_density_from_temperature
  const Real _density_reference_pressure;

  /// Conversion of unit_weight_temperature's values to Kelvin (0 for Kelvin, 273.15 for Celsius)
  const Real _t_c2k;

  /**
   * Wellbore pressure at each well point, indexed by Dirac point ID.  Only used if
   * _use_density_from_temperature.  Recomputed once per residual evaluation and once per
   * Jacobian evaluation (see residualSetup()/jacobianSetup()) so that the residual always uses
   * the fluid density implied by the current nonlinear iterate's temperature.
   */
  std::vector<Real> _bh_pressure;

  /**
   * (Re)computes _bh_pressure from the temperature at each well point, when
   * _use_density_from_temperature is true.  Does nothing otherwise.
   */
  void computeWellborePressures();

  /**
   * The wellbore pressure (or temperature, for function_of=temperature) at the given Dirac
   * point, ie P_bot + unit_weight.(x_i - x_bottom) in the constant-unit_weight case, or the
   * temperature-dependent-density-integrated profile otherwise.
   */
  Real wellborePressure(unsigned current_dirac_ptid) const;

  /// Borehole constant
  const Real _re_constant;

  /// Well constant
  const Real _well_constant;

  /// Whether there is a quadpoint permeability material (for error checking)
  const bool _has_permeability;

  /// Whether there is a quadpoint thermal conductivity material (for error checking)
  const bool _has_thermal_conductivity;

  /// Permeability or conductivity of porous material
  const MaterialProperty<RealTensorValue> & _perm_or_cond;

  /// d(Permeability)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dperm_or_cond_dvar;

  /// Rotation matrix used in well_constant calculation
  std::vector<RealTensorValue> _rot_matrix;

  /**
   * Calculates Peaceman's form of the borehole well constant
   * Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and
   * Modeling, 3 (2008) 375-388
   */
  Real wellConstant(const RealTensorValue & perm,
                    const RealTensorValue & rot,
                    const Real & half_len,
                    const Elem * ele,
                    const Real & rad) const;

  Real computeQpBaseOutflow(unsigned current_dirac_ptid) const override;
  void computeQpBaseOutflowJacobian(unsigned jvar,
                                    unsigned current_dirac_ptid,
                                    Real & outflow,
                                    Real & outflowp) const override;
};
