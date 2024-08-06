// 5D Gaussian anisotropy material object
// Material properties adding anisotropy to epsilon [or also called k] and m;
// L is isotropic and computed from a given mobility.

#pragma once

#include "ADMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"
#include <vector>

class EpsAndMAnisoGauss : public ADMaterial
{
public:
  static InputParameters validParams();

  EpsAndMAnisoGauss(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  void readQuaternionFile();
  void readLibraryFile();

  // Grains orientation quaternions.
  std::vector<double> _qwR, _qxR, _qyR, _qzR;
  // Normalized_Axis_X  Normalized_Axis_Y Normalized_Axis_Z  Angle(Radian) GB_Normal_X  GB_Normal_Y
  // GB_Normal_Z  Minimun_GB_energy(J/m^2).
  std::vector<double> _vbalibx, _vbaliby, _vbalibz, _thetabalib, _miubalibx, _miubaliby, _miubalibz,
      _MinimaEnergy;

  // Names of material properties.
  const MaterialPropertyName _eps_name;        // The name of the anisotropic epsilon - AD.
  const MaterialPropertyName _eps_EN_name;     // The name of the anisotropic epsilon - NoAD.
  const MaterialPropertyName _deps_minus_name; // Negative derivatives of epsilon - AD.
  const MaterialPropertyName _deps_plus_name;  // Positive derivatives of epsilon - AD.
  const MaterialPropertyName
      _VlibR_name; // The name of the library boundary normals in the simulation reference frame.
  const MaterialPropertyName
      _Vec_name; // The name of the normalized gradients in the simulation reference frame.
  const MaterialPropertyName _dm_minus_name;     // Negative derivatives of m - AD.
  const MaterialPropertyName _dm_plus_name;      // Positive derivatives of m - AD.
  const MaterialPropertyName _S_switch_name;     // The name of the switch property.
  const MaterialPropertyName _m_name;            // The name of the anisotropic m - AD.
  const MaterialPropertyName _m_EN_name;         // The name of the anisotropic m - NoAD.
  const MaterialPropertyName _L_name;            // The name of L - AD.
  const MaterialPropertyName _L_EN_name;         // The name of L - NoAD.
  const MaterialPropertyName _sigma_name;        // Grain boundary energy in eV/nm^2.
  const MaterialPropertyName _sigmaORIUNIT_name; // Grain boundary energy in J/m^2.
  const MaterialPropertyName _qwg_name;          // w component of the quaternion.
  const MaterialPropertyName _qxg_name;          // x component of the quaternion.
  const MaterialPropertyName _qyg_name;          // y component of the quaternion.
  const MaterialPropertyName _qzg_name;          // z component of the quaternion.

  // Epsilon.
  ADMaterialProperty<Real> & _eps;
  MaterialProperty<Real> & _eps_EN;

  // Derivatives of epsilon.
  ADMaterialProperty<RealGradient> & _deps_minus;
  ADMaterialProperty<RealGradient> & _deps_plus;

  // Switch for anisotropy calculation.
  ADMaterialProperty<Real> & _S_switch;

  // Library boundary normals in the simulation reference frame.
  ADMaterialProperty<RealGradient> & _VlibR;

  // Normalized gradients in the simulation reference frame.
  ADMaterialProperty<RealGradient> & _Vec;

  // Derivatives of m.
  ADMaterialProperty<RealGradient> & _dm_minus;
  ADMaterialProperty<RealGradient> & _dm_plus;

  // Whether to add Gaussian to epsilon.
  const bool & _ADDGaussian;

  //  m.
  ADMaterialProperty<Real> & _m;
  MaterialProperty<Real> & _m_EN;

  // Mobility property.
  ADMaterialProperty<Real> & _L;
  MaterialProperty<Real> & _L_EN;

  // Grain boundary energy properties eV/nm^2 and J//m^2.
  ADMaterialProperty<Real> & _sigma;
  ADMaterialProperty<Real> & _sigmaORIUNIT;

  // Quaternion components
  ADMaterialProperty<Real> & _qwg;
  ADMaterialProperty<Real> & _qxg;
  ADMaterialProperty<Real> & _qyg;
  ADMaterialProperty<Real> & _qzg;

  // Constants for calculation. Base energy and L.
  const Real _sigmaBASE;

  // Gaussian calculation parameters.
  const Real _Gaussian_Tolerance;
  const Real _gamma;
  const Real _lgb;
  const Real _alphaswitch;
  const Real _betaswitch;
  unsigned int _libnum;
  const Real _amplitudeScale;
  const Real _sharpness;

  // File names for library and quaternion data.
  const FileName _Library_file_name;
  const FileName _Quaternion_file_name;

  // Grain tracker interface.
  const GrainTrackerInterface & _grain_tracker;

  // Scaling factors for length and time and energy.
  const Real _length_scale;
  const Real _time_scale;
  const Real _JtoeV;

  // Constants for calculation of L.
  const Real _GBMobility;
  const Real _Mob;
  const Real _Q;
  const Real _T;
  const Real _kb;

  // Boundary normal computation flag.
  const Real _BoundaryNormal;

  // Number of coupled variables.
  const unsigned int _op_num;

  // Current grain number.
  unsigned int _grain_num;

  // Coupled variable values and gradients.
  const std::vector<const ADVariableValue *> _vals;
  const std::vector<const ADVariableGradient *> _grad_vals;
};
