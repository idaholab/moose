
// 5D Gaussian anisotropy material object
// Material properties adding anysotroopy to ε [or also called k] and m;
// L  is isotropic and computed from a given mobility.


#pragma once

#include "ADMaterial.h"

class GrainTrackerInterface;

class EPSandManisoGAUSS : public ADMaterial
{
public:
  static InputParameters validParams();

  EPSandManisoGAUSS(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  ADMaterialProperty<Real> & _eps;

  MaterialProperty<Real> & _epsenergy;

  ADMaterialProperty<Real> & _depsdx;

  ADMaterialProperty<Real> & _depsdy;

  ADMaterialProperty<Real> & _depsdz;

  MaterialProperty<Real> & _depsdx_EN;

  MaterialProperty<Real> & _depsdy_EN;

  MaterialProperty<Real> & _depsdz_EN;

  ADMaterialProperty<Real> & _epsCalc;
  ADMaterialProperty<Real> & _mCalc;
  const MaterialPropertyName _epsCalc_name;
  const MaterialPropertyName _mCalc_name;

  ADMaterialProperty<Real> & _depsdxplus;
  ADMaterialProperty<Real> & _depsdyplus;
  ADMaterialProperty<Real> & _depsdzplus;
  MaterialProperty<Real> & _depsdxplus_EN;
  MaterialProperty<Real> & _depsdyplus_EN;
  MaterialProperty<Real> & _depsdzplus_EN;

  ADMaterialProperty<Real> & _S_switch;
  const MaterialPropertyName _S_switch_name;

  ADMaterialProperty<Real> & _VwlibR;
  ADMaterialProperty<Real> & _VxlibR;
  ADMaterialProperty<Real> & _VylibR;
  ADMaterialProperty<Real> & _VzlibR;

  ADMaterialProperty<Real> & _Vx;
  ADMaterialProperty<Real> & _Vy;
  ADMaterialProperty<Real> & _Vz;

  ADMaterialProperty<Real> & _dmdx;
  ADMaterialProperty<Real> & _dmdy;
  ADMaterialProperty<Real> & _dmdz;
  MaterialProperty<Real> & _dmdx_EN;
  MaterialProperty<Real> & _dmdy_EN;
  MaterialProperty<Real> & _dmdz_EN;
  ADMaterialProperty<Real> & _dmdxplus;
  ADMaterialProperty<Real> & _dmdyplus;
  ADMaterialProperty<Real> & _dmdzplus;
  MaterialProperty<Real> & _dmdxplus_EN;
  MaterialProperty<Real> & _dmdyplus_EN;
  MaterialProperty<Real> & _dmdzplus_EN;

  const bool & _ADDGaussian;

  ADMaterialProperty<Real> & _m;
  MaterialProperty<Real> & _m_energy;

  ADMaterialProperty<Real> & _L;
  MaterialProperty<Real> & _L_energy;

  ADMaterialProperty<Real> & _sigma;
  ADMaterialProperty<Real> & _sigmaORIUNIT;

  ADMaterialProperty<Real> & _qwg;

  ADMaterialProperty<Real> & _qxg;

  ADMaterialProperty<Real> & _qyg;

  ADMaterialProperty<Real> & _qzg;

  ADMaterialProperty<Real> & _epsbar;
  ADMaterialProperty<Real> & _epsmin2grains;

  ADMaterialProperty<Real> & _TotGauss;

  const Real  _sigmaBASE;

  const MaterialPropertyName _eps_name;
  const MaterialPropertyName _epsenergy_name;

  const MaterialPropertyName _depsdx_name;

  const MaterialPropertyName _depsdy_name;

  const MaterialPropertyName _depsdz_name;

  const MaterialPropertyName _m_name;
  const MaterialPropertyName _m_energy_name;

  const MaterialPropertyName _L_name;
  const MaterialPropertyName _L_energy_name;

  const Real  _Gaussian_Tolerance;

  const Real  _gamma;

  const Real  _lgb;

  const Real  _alphaswitch;

  const Real  _betaswitch;

  unsigned int  _libnum;

  const Real  _amplitudeScale;

  const Real  _sharpness;

  const FileName _Library_file_name;

  const FileName _Quaternion_file_name;

  const GrainTrackerInterface & _grain_tracker;

  const Real _length_scale;

  const Real _time_scale;

  const Real _JtoeV;

  const Real  _GBMobility;

  const Real  _Mob;

  const Real  _Q;

  const Real  _T;

  const Real  _kb;

  const Real  _BoundaryNormal;

  const unsigned int _op_num;

        unsigned int _grain_num;

  const std::vector<const ADVariableValue *> _vals;

  const std::vector<const ADVariableGradient *> _grad_vals;


};
