//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARAMETRICMATERIALBASEPD_H
#define PARAMETRICMATERIALBASEPD_H

#include "MechanicsMaterialBasePD.h"
#include "RankFourTensor.h"

class ParametricMaterialBasePD;

template <>
InputParameters validParams<ParametricMaterialBasePD>();

/**
 * Base material class for bond-based and ordinary state-based peridynamic models, i.e. parameteric
 * material models
 */
class ParametricMaterialBasePD : public MechanicsMaterialBasePD
{
public:
  ParametricMaterialBasePD(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Function to compute the temperature at current two material points
   */
  void computeNodalTemperature();

  virtual void computeBondStretch() override;

  /**
   * Function to compute the interaction force for a bond
   */
  virtual void computeBondForce() = 0;

  /// Plane stress model or not
  const bool _plane_stress;

  const Real _youngs_modulus;
  const Real _poissons_ratio;

  ///@{ Scalar out-of-plane component of strain tensor for generalized plane strain
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  /// Temperature variable
  std::vector<Real> _temp;

  /// Reference temperature
  const Real _temp_ref;

  /// Thermal expension coefficient
  Real _alpha;

  ///@{ Material properties to store
  MaterialProperty<Real> & _bond_force_ij;
  MaterialProperty<Real> & _bond_dfdU_ij;
  MaterialProperty<Real> & _bond_dfdT_ij;
  MaterialProperty<Real> & _bond_dfdE_ij;

  MaterialProperty<RankFourTensor> & _elasticity_tensor;
  MaterialProperty<Real> & _thermal_expansion_coeff;
  ///@}

  Real _shear_modulus;
  Real _bulk_modulus;

  /// Elasticity tensor
  RankFourTensor _Cijkl;
};

#endif // PARAMETRICMATERIALBASEPD_H
