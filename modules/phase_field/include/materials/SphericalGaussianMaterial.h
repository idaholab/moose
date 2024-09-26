//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"
#include <vector>

/**
 * 5D-gaussian-anisotropy material object for anisotropic grain growth
 * This material adds anisotropy to epsilon - also called kappa, m - also called mu, and L for epsilon model or adds anisotropy to gamma and L for gamma model
 */

class SphericalGaussianMaterial : public ADMaterial
{
public:
  static InputParameters validParams();

  SphericalGaussianMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  /// Type of models
  enum class ModelType
  {
    EPSILON,
    GAMMA
  };

  /// Choose between the epsilon and gamma models
  const ModelType _model_type;

  ///@{ Read and store grains orientation quaternions data and minima misorientations (minima library) data from text files
  void readQuaternionFile();
  void readLibraryFile();
  ///@}

  ///@{ Grains orientation quaternions - compnents: 0,1,2,3. Original values in library file can be given in normalized form or not
  std::vector<ADReal> _q0;
  std::vector<ADReal> _q1;
  std::vector<ADReal> _q2;
  std::vector<ADReal> _q3;
  ///@}

  /// Axis of rotation for minima library misorientation. Original values in library file can be given in normalized form or not
  std::vector<ADRealVectorValue> _library_rotation_axis;

  /// Angle of rotation for minima library misorientation (Radian)
  std::vector<ADReal> _library_rotation_angle;

  /// Library gaussian direction. Original values in library file can be given in normalized form or not
  std::vector<ADRealVectorValue> _library_gaussian_direction;

  /// Positive grain boundary energy for minima library misorientation (J/m^2)
  std::vector<ADReal> _library_minima_energy;

  /// Positive pre-determined value of L for minima library misorientation (nm^3/eV.ns)
  std::vector<ADReal> _library_minima_L;

  ///@{ Names of the file containing orientation quaternions data and minima misorientations (minima library) data
  const FileName _quaternion_file_name;
  const FileName _library_file_name;
  ///@}

  ///@{ Names of material properties
  const MaterialPropertyName _epsilon_name;
  const MaterialPropertyName _m_name;
  const MaterialPropertyName _mob_L_name;
  const MaterialPropertyName _gamma_name;
  const MaterialPropertyName _sigmaORIUNIT_name;
  ///@}

  ///@{ Material properties
  ADMaterialProperty<Real> & _epsilon;
  ADMaterialProperty<Real> & _m;
  ADMaterialProperty<Real> & _mob_L;
  ADMaterialProperty<Real> & _gamma;
  ADMaterialProperty<Real> & _sigmaORIUNIT;
  ///@}

  ///@{ Derivatives of material properties
  ADMaterialProperty<RealGradient> & _depsilon_plus;
  ADMaterialProperty<RealGradient> & _depsilon_minus;
  ADMaterialProperty<RealGradient> & _dm_plus;
  ADMaterialProperty<RealGradient> & _dm_minus;
  ADMaterialProperty<RealGradient> & _dgamma_plus;
  ADMaterialProperty<RealGradient> & _dgamma_minus;
  ///@}

  ///@{ Value of gamma used to specify an isotropic material propertie gamma and its associated g_gamma and f_gamma values (dimensionless)
  const Real _gamma_value;
  const Real _g_gamma_value;
  const Real _f_gamma_value;
  ///@}

  /// Grain boundary width (nm)
  const Real _grain_boundary_width;

  ///@{ Base values from which gaussian is either added or substracted
  const Real _base_energy;
  const Real _base_gamma;
  const Real _base_g_gamma;
  const Real _base_f_gamma;
  const Real _base_L;
  ///@}

  /// Value used to control gaussian effect
  const Real _gaussian_tolerance;

  ///@{ Acceptable ranges for axis and angle variance for gaussian switch calculation (Radian)
  const Real _axis_variance_range;
  const Real _angle_variance_range;
  ///@}

  /// Gaussian sharpness (dimensionless)
  const Real _sharpness;

  ///@{ Bools to specify anisotropy (True) or isotropy (false) of material properties
  const bool & _anisotropic_epsilon;
  const bool & _anisotropic_m;
  const bool & _anisotropic_gamma;
  const bool & _anisotropic_L;
  ///@}

  ///@{ Bools to specify whether the gaussian is added (True) or sustracted (false) to the base value in gaussian direction
  const bool & _add_epsilon_gaussian;
  const bool & _add_g_gamma_gaussian;
  const bool & _add_L_gaussian;
  ///@}

  /// Bool to determine if the final gaussian direction is computed using orientation quaternions (True) or taken directly from library file (false)
  const bool & _compute_final_gaussian_direction;

  /// Bool to determine if the anisotropic material properties are at base values within the grains (True) or at zero (false)
  const bool & _set_bulk_to_base_values;

  /// Number of minima misorientations in the minima library file to use
  std::size_t _library_misorientations_number;

  /// Number of grains
  std::size_t _grain_num;

  /// Number of coupled order parameter variables
  const std::size_t _op_num;

  ///@{ Coupled order parameter variables values and gradients
  const std::vector<const ADVariableValue *> _vals;
  const std::vector<const ADVariableGradient *> _grad_vals;
  ///@}

  /// Grain tracker UserObject
  const GrainTrackerInterface & _grain_tracker;

  ///@{ Scaling factors for length, time and energy
  const Real _length_scale;
  const Real _time_scale;
  const Real _energy_scale;
  ///@}
};
