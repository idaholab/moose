//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLineSink.h"

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

  /// Unit weight of fluid in borehole (for calculating bottomhole pressure at each Dirac Point)
  const RealVectorValue _unit_weight;

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
