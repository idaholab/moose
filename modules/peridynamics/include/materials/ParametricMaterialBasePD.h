//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsMaterialBasePD.h"
#include "RankFourTensor.h"

/**
 * Base material class for bond-based and ordinary state-based peridynamic models, i.e. parameteric
 * material models
 */
class ParametricMaterialBasePD : public MechanicsMaterialBasePD
{
public:
  static InputParameters validParams();

  ParametricMaterialBasePD(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeBondStretch() override;

  /**
   * Function to compute force of a bond
   */
  virtual void computeBondForce() = 0;

  /**
   * Function to compute material constants from elasticity tensor
   */
  void computeMaterialConstants();

  /**
   * Function to compute the micro-moduli for bond-based and ordinary state-based models
   */
  virtual void computePeridynamicsParams() = 0;

  /// Plane stress problem or not
  const bool _plane_stress;

  ///@{ Scalar out-of-plane component of strain tensor for generalized plane strain
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  /// Temperature variable
  std::vector<Real> _temp;

  /// Reference temperature
  const Real _temp_ref;

  /// Thermal expension coefficient
  Real _tec;
  Real _alpha;

  ///@{ Material properties to store
  MaterialProperty<Real> & _bond_local_force;
  MaterialProperty<Real> & _bond_local_dfdU;
  MaterialProperty<Real> & _bond_local_dfdT;
  MaterialProperty<Real> & _bond_local_dfdE;

  MaterialProperty<Real> & _thermal_expansion_coeff;
  ///@}

  /// Material properties to fetch
  const MaterialProperty<RankFourTensor> & _Cijkl;

  Real _youngs_modulus;
  Real _poissons_ratio;
  Real _shear_modulus;
  Real _bulk_modulus;
};
