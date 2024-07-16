
// 5D Gaussian anisotropy material object
// Material properties adding anisotropy to gamma and L.

#pragma once

#include "ADMaterial.h"

class GrainTrackerInterface;

class GammaAndLAnisoGauss : public ADMaterial
{
public:
  static InputParameters validParams();

  GammaAndLAnisoGauss(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  ADMaterialProperty<Real> & _Ggamma;
  MaterialProperty<Real> & _Ggamma_EN;

  ADMaterialProperty<Real> & _dGgammadx;
  MaterialProperty<Real> & _dGgammadx_EN;
  ADMaterialProperty<Real> & _dGgammady;
  MaterialProperty<Real> & _dGgammady_EN;
  ADMaterialProperty<Real> & _dGgammadz;
  MaterialProperty<Real> & _dGgammadz_EN;

  ADMaterialProperty<Real> & _dGgammadxplus;
  MaterialProperty<Real> & _dGgammadxplus_EN;
  ADMaterialProperty<Real> & _dGgammadyplus;
  MaterialProperty<Real> & _dGgammadyplus_EN;
  ADMaterialProperty<Real> & _dGgammadzplus;
  MaterialProperty<Real> & _dGgammadzplus_EN;

  ADMaterialProperty<Real> & _VwlibR;
  ADMaterialProperty<Real> & _VxlibR;
  ADMaterialProperty<Real> & _VylibR;
  ADMaterialProperty<Real> & _VzlibR;

  ADMaterialProperty<Real> & _Vx;
  ADMaterialProperty<Real> & _Vy;
  ADMaterialProperty<Real> & _Vz;

  ADMaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _gamma_EN;

  ADMaterialProperty<Real> & _S_switch;

  ADMaterialProperty<Real> & _dgammadx;
  MaterialProperty<Real> & _dgammadx_EN;
  ADMaterialProperty<Real> & _dgammady;
  MaterialProperty<Real> & _dgammady_EN;
  ADMaterialProperty<Real> & _dgammadz;
  MaterialProperty<Real> & _dgammadz_EN;

  ADMaterialProperty<Real> & _dgammadxplus;
  MaterialProperty<Real> & _dgammadxplus_EN;
  ADMaterialProperty<Real> & _dgammadyplus;
  MaterialProperty<Real> & _dgammadyplus_EN;
  ADMaterialProperty<Real> & _dgammadzplus;
  MaterialProperty<Real> & _dgammadzplus_EN;

  ADMaterialProperty<Real> & _m;
  MaterialProperty<Real> & _m_EN;

  ADMaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _kappa_EN;

  ADMaterialProperty<Real> & _L;
  MaterialProperty<Real> & _L_EN;

  ADMaterialProperty<Real> & _lgb;
  MaterialProperty<Real> & _lgb_EN;

  const bool & _ADDGaussian;

  const bool & _ADDGaussianL;

  ADMaterialProperty<Real> & _sigma;
  MaterialProperty<Real> & _sigma_EN;
  ADMaterialProperty<Real> & _sigmaORIUNIT;
  MaterialProperty<Real> & _sigmaORIUNIT_EN;

  ADMaterialProperty<Real> & _qwg;

  ADMaterialProperty<Real> & _qxg;

  ADMaterialProperty<Real> & _qyg;

  ADMaterialProperty<Real> & _qzg;

  ADMaterialProperty<Real> & _Ggammabar;

  ADMaterialProperty<Real> & _Ggammamin2grains;

  ADMaterialProperty<Real> & _TotGauss;

  ADMaterialProperty<Real> & _MGBVALUE;

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

  const FileName _Library_file_name;

  const FileName _Quaternion_file_name;

  const GrainTrackerInterface & _grain_tracker;

  const Real _length_scale;

  const Real _time_scale;

  const Real _JtoeV;

  const Real _BoundaryNormal;

  const unsigned int _op_num;

  const std::vector<const ADVariableValue *> _vals;

  const std::vector<const ADVariableGradient *> _grad_vals;

  const MaterialPropertyName _Ggamma_name;
  const MaterialPropertyName _Ggamma_EN_name;

  const MaterialPropertyName _dGgammadx_name;
  const MaterialPropertyName _dGgammadx_EN_name;

  const MaterialPropertyName _dGgammady_name;
  const MaterialPropertyName _dGgammady_EN_name;

  const MaterialPropertyName _dGgammadz_name;
  const MaterialPropertyName _dGgammadz_EN_name;

  const MaterialPropertyName _dGgammadxplus_name;
  const MaterialPropertyName _dGgammadxplus_EN_name;
  const MaterialPropertyName _dGgammadyplus_name;
  const MaterialPropertyName _dGgammadyplus_EN_name;
  const MaterialPropertyName _dGgammadzplus_name;
  const MaterialPropertyName _dGgammadzplus_EN_name;

  const MaterialPropertyName _gamma_name;
  const MaterialPropertyName _gamma_EN_name;

  const MaterialPropertyName _S_switch_name;

  const MaterialPropertyName _dgammadx_name;
  const MaterialPropertyName _dgammadx_EN_name;

  const MaterialPropertyName _dgammady_name;
  const MaterialPropertyName _dgammady_EN_name;

  const MaterialPropertyName _dgammadz_name;
  const MaterialPropertyName _dgammadz_EN_name;

  const MaterialPropertyName _dgammadxplus_name;
  const MaterialPropertyName _dgammadxplus_EN_name;
  const MaterialPropertyName _dgammadyplus_name;
  const MaterialPropertyName _dgammadyplus_EN_name;
  const MaterialPropertyName _dgammadzplus_name;
  const MaterialPropertyName _dgammadzplus_EN_name;

  const MaterialPropertyName _m_name;
  const MaterialPropertyName _m_EN_name;

  const MaterialPropertyName _kappa_name;
  const MaterialPropertyName _kappa_EN_name;

  const MaterialPropertyName _L_name;
  const MaterialPropertyName _L_EN_name;

  const MaterialPropertyName _lgb_name;
  const MaterialPropertyName _lgb_EN_name;

  unsigned int _grain_num;
};
