// 5D Gaussian anisotropy material object
// Material properties adding anisotropy to gamma and L.

#pragma once

#include "ADMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"
#include <vector>

class GammaAndLAnisoGauss : public ADMaterial
{
public:
  static InputParameters validParams();
  GammaAndLAnisoGauss(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  void readQuaternionFile();
  void readLibraryFile();

  // Grains orientation quaternions.
  std::vector<double> _qwR, _qxR, _qyR, _qzR;
  // Normalized_Axis_X  Normalized_Axis_Y Normalized_Axis_Z  Angle(Radian) GB_Normal_X  GB_Normal_Y
  // GB_Normal_Z  Minimun_GB_energy(J/m^2) Minimun_L(nm^3/eV.ns).
  std::vector<double> _vbalibx, _vbaliby, _vbalibz, _thetabalib, _miubalibx, _miubaliby, _miubalibz,
      _MinimaEnergy, _MinimaL;

  // Names of material properties.
  const MaterialPropertyName _Ggamma_name;        // The name of the anisotropic Ggamma - AD.
  const MaterialPropertyName _dGgamma_minus_name; // Negative derivatives of Ggamma - AD.
  const MaterialPropertyName _dGgamma_plus_name;  // Positive derivatives of Ggamma - AD.
  const MaterialPropertyName _gamma_name;         // The name of anisotropic gamma - AD.
  const MaterialPropertyName _gamma_EN_name;      // The name of anisotropic gamma - NoAD.
  const MaterialPropertyName _dgamma_minus_name;  // Negative derivatives of gamma - AD.
  const MaterialPropertyName _dgamma_plus_name;   // Positive derivatives of gamma - AD.
  const MaterialPropertyName _S_switch_name;      // The name of the switch property.
  const MaterialPropertyName _m_name;             // The name of m - AD.
  const MaterialPropertyName _m_EN_name;          // The name of m - NoAD.
  const MaterialPropertyName _kappa_name;         // The name of kappa - AD.
  const MaterialPropertyName _kappa_EN_name;      // The name of kappa - NoAD.
  const MaterialPropertyName _L_name;             // The name of the anisotropic L - AD.
  const MaterialPropertyName _L_EN_name;          // The name of the anisotropic L - NoAD.
  const MaterialPropertyName _lgb_name; // The name of anisotropic lgb (grain boundary width) - AD.
  const MaterialPropertyName
      _lgb_EN_name; // The name of anisotropic lgb (grain boundary width) - NoAD.
  const MaterialPropertyName _sigma_name;        // Grain boundary energy in eV/nm^2.
  const MaterialPropertyName _sigmaORIUNIT_name; // Grain boundary energy in J//m^2.
  const MaterialPropertyName _qwg_name;          // w component of quaternion.
  const MaterialPropertyName _qxg_name;          // x component of quaternion.
  const MaterialPropertyName _qyg_name;          // y component of quaternion.
  const MaterialPropertyName _qzg_name;          // z component of quaternion.

  // Ggamma.
  ADMaterialProperty<Real> & _Ggamma;

  // Derivatives of Ggamma.
  ADMaterialProperty<RealGradient> & _dGgamma_minus;
  ADMaterialProperty<RealGradient> & _dGgamma_plus;

  // Library boundary normals in the simulation reference frame.
  ADMaterialProperty<RealGradient> & _VlibR;

  // Normalized gradients in the simulation reference frame.
  ADMaterialProperty<RealGradient> & _Vec;

  // gamma
  ADMaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _gamma_EN;

  // Derivatives of gamma.
  ADMaterialProperty<RealGradient> & _dgamma_minus;
  ADMaterialProperty<RealGradient> & _dgamma_plus;

  // Switch for anisotropy calculation.
  ADMaterialProperty<Real> & _S_switch;

  // Whether to add Gaussian to epsilon.
  const bool & _ADDGaussian;

  // Whether to add Gaussian to L.
  const bool & _ADDGaussianL;

  //  m.
  ADMaterialProperty<Real> & _m;
  MaterialProperty<Real> & _m_EN;

  // Epsilon [or also called kappa].
  ADMaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _kappa_EN;

  // Mobility property.
  ADMaterialProperty<Real> & _L;
  MaterialProperty<Real> & _L_EN;

  //  lgb - grain boundary  width..
  ADMaterialProperty<Real> & _lgb;
  MaterialProperty<Real> & _lgb_EN;

  // Grain boundary energy properties eV/nm^2 and J//m^2..
  ADMaterialProperty<Real> & _sigma;
  ADMaterialProperty<Real> & _sigmaORIUNIT;

  // Quaternion components..
  ADMaterialProperty<Real> & _qwg;
  ADMaterialProperty<Real> & _qxg;
  ADMaterialProperty<Real> & _qyg;
  ADMaterialProperty<Real> & _qzg;

  // Gaussian calculation parameters..
  const Real _sigmaBASE;
  const Real _GgammaBASE;
  const Real _gammaBASE;
  const Real _f0gammaBASE;
  const Real _L_BASE;
  const Real _lgbBASE_minimum;
  const Real _alphaswitch;
  const Real _betaswitch;
  unsigned int _libnum;
  const Real _amplitudeScale;
  const Real _sharpness;
  const Real _Gaussian_Tolerance;

  // File names for library and quaternion data.
  const FileName _Library_file_name;
  const FileName _Quaternion_file_name;

  // Grain tracker interface.
  const GrainTrackerInterface & _grain_tracker;

  // Scaling factors for length and time and energy.
  const Real _length_scale;
  const Real _time_scale;
  const Real _JtoeV;

  // Boundary normal computation flag.
  const Real _BoundaryNormal;

  // Number of coupled variables.
  const unsigned int _op_num;

  // Coupled variable values and gradients.
  const std::vector<const ADVariableValue *> _vals;
  const std::vector<const ADVariableGradient *> _grad_vals;

  // Current grain number.
  unsigned int _grain_num;
};
