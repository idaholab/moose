// 5D Gaussian anisotropy material object
// Material properties adding anisotropy to gamma;
// L is isotropic and computed from a given mobility.

#include "GammaAnisoGauss.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

registerMooseObject("PhaseFieldApp", GammaAnisoGauss);

InputParameters
GammaAnisoGauss::validParams()
{
  InputParameters params = ADMaterial::validParams();

  params.addClassDescription("Material to provide an anisotropy field and its derivatives with "
                             "respect to the gradient of the order parameter");

  params.addParam<MaterialPropertyName>(
      "Ggamma_name", "Ggamma", "The name of the anisotropic Ggamma - AD.");
  params.addParam<MaterialPropertyName>(
      "dGgamma_minus_name", "dGgamma_minus", "Negative derivatives of Ggamma - AD.");
  params.addParam<MaterialPropertyName>(
      "dGgamma_plus_name", "dGgamma_plus", "Positive derivatives of Ggamma - AD.");
  params.addParam<MaterialPropertyName>(
      "VlibR_name",
      "VlibR",
      "The name of the library boundary normals in the simulation reference frame.");
  params.addParam<MaterialPropertyName>(
      "Vec_name", "Vec", "The name of the normalized gradients in the simulation reference frame.");
  params.addParam<MaterialPropertyName>(
      "gamma_name", "gamma", "The name of anisotropic gamma - AD.");
  params.addParam<MaterialPropertyName>(
      "gamma_EN_name", "gamma_asymm", "The name of anisotropic gamma - NoAD.");
  params.addParam<MaterialPropertyName>(
      "dgamma_minus_name", "dgamma_minus", "Negative derivatives of gamma - AD.");
  params.addParam<MaterialPropertyName>(
      "dgamma_plus_name", "dgamma_plus", "Positive derivatives of gamma - AD.");
  params.addParam<MaterialPropertyName>(
      "S_switch_name", "S_switch", "The name of the switch property.");
  params.addParam<MaterialPropertyName>("m_name", "m", "The name of m - AD.");
  params.addParam<MaterialPropertyName>("m_EN_name", "mu", "The name of m - NoAD.");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa", "The name of kappa - AD.");
  params.addParam<MaterialPropertyName>("kappa_EN_name", "kappa_op", "The name of kappa - NoAD.");
  params.addParam<MaterialPropertyName>("L_name", "L_AD", "The name of L - AD.");
  params.addParam<MaterialPropertyName>("L_EN_name", "L", "The name of L - NoAD.");
  params.addParam<MaterialPropertyName>(
      "lgb_name", "lgb", "The name of anisotropic lgb (grain boundary width) - AD.");
  params.addParam<MaterialPropertyName>(
      "lgb_EN_name", "lgb_EN", "The name of anisotropic lgb (grain boundary width) - NoAD.");
  params.addParam<MaterialPropertyName>("sigma_name", "sigma", "Grain boundary energy in eV/nm^2.");
  params.addParam<MaterialPropertyName>(
      "sigmaORIUNIT_name", "sigmaORIUNIT", "Grain boundary energy in J//m^2.");
  params.addParam<MaterialPropertyName>("qwg_name", "qwg_name", "w component of quaternion.");
  params.addParam<MaterialPropertyName>("qxg_name", "qxg_name", "x component of quaternion.");
  params.addParam<MaterialPropertyName>("qyg_name", "qyg_name", "y component of quaternion.");
  params.addParam<MaterialPropertyName>("qzg_name", "qzg_name", "z component of quaternion.");
  params.addRequiredParam<FileName>("Library_file_name",
                                    "Name of the file containing anisotropic data.");
  params.addRequiredParam<FileName>("Quaternion_file_name",
                                    "Name of the file containing quaternion data.");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables.");
  params.addRequiredParam<Real>("sigmaBASE", "Value of sigma base energy in J/m^2.");
  params.addRequiredParam<Real>("gammaBASE", "Value of gamma at sigmaBASE energy.");
  params.addRequiredParam<Real>("GgammaBASE", "Value of Ggamma at gammaBASE energy.");
  params.addRequiredParam<Real>("f0gammaBASE", "Value of f0gamma at gammaBASE energy.");
  params.addRequiredParam<Real>("lgbBASE_minimum", "Value of minimum grain boundary width in n.m");
  params.addRequiredParam<Real>("alphaswitch", "Acceptable range of angle from vbalib.");
  params.addRequiredParam<Real>("betaswitch", "Acceptable range of angle from thetabalib.");
  params.addRequiredParam<int>("libnum", "Number of misorientations in the library.");
  params.addRequiredParam<Real>("amplitudeScale", "Amplitude scale of gaussians.");
  params.addRequiredParam<Real>("sharpness", "Sharpness of gaussians.");
  params.addRequiredParam<Real>("Gaussian_Tolerance", "Gaussian Tolerance.");
  params.addRequiredParam<UserObjectName>("grain_tracker", "Graintracker UserObject.");
  params.addRequiredParam<bool>("ADDGaussian",
                                "If this is true gaussians add to gammaBASE, else subtract.");
  params.addParam<Real>("length_scale", 1.0e-9, "Conversion for 1/m to 1/nm.");
  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns.");
  params.addParam<Real>("JtoeV", 6.24150974e18, "Joule to eV conversion.");
  params.addParam<Real>(
      "GBMobility",
      1,
      "GB mobility input in m^4/(J*s), that overrides the temperature dependent calculation.");
  params.addParam<Real>("Mob", 0, "Grain boundary mobility prefactor in m^4/(J*s).");
  params.addParam<Real>("Q", 0, "Grain boundary migration activation energy in eV.");
  params.addParam<Real>("T", 273, "Temperature in Kelvin.");
  params.addParam<Real>("kb", 8.617343e-5, "Boltzmann constant in eV/K.");
  params.addParam<Real>(
      "BoundaryNormal",
      0,
      "If 0, the boundary normal in the simulation reference frame is computed using the "
      "orientation quaternions, else it is taken directly from the minima library file.");

  return params;
}

GammaAnisoGauss::GammaAnisoGauss(const InputParameters & parameters)
  : ADMaterial(parameters),
    _Ggamma(declareADProperty<Real>(getParam<MaterialPropertyName>("Ggamma_name"))),
    _dGgamma_minus(
        declareADProperty<RealGradient>(getParam<MaterialPropertyName>("dGgamma_minus_name"))),
    _dGgamma_plus(
        declareADProperty<RealGradient>(getParam<MaterialPropertyName>("dGgamma_plus_name"))),
    _VlibR(declareADProperty<RealGradient>(getParam<MaterialPropertyName>("VlibR_name"))),
    _Vec(declareADProperty<RealGradient>(getParam<MaterialPropertyName>("Vec_name"))),
    _gamma(declareADProperty<Real>(getParam<MaterialPropertyName>("gamma_name"))),
    _gamma_EN(declareProperty<Real>(getParam<MaterialPropertyName>("gamma_EN_name"))),
    _dgamma_minus(
        declareADProperty<RealGradient>(getParam<MaterialPropertyName>("dgamma_minus_name"))),
    _dgamma_plus(
        declareADProperty<RealGradient>(getParam<MaterialPropertyName>("dgamma_plus_name"))),
    _S_switch(declareADProperty<Real>(getParam<MaterialPropertyName>("S_switch_name"))),
    _ADDGaussian(getParam<bool>("ADDGaussian")),
    _m(declareADProperty<Real>(getParam<MaterialPropertyName>("m_name"))),
    _m_EN(declareProperty<Real>(getParam<MaterialPropertyName>("m_EN_name"))),
    _kappa(declareADProperty<Real>(getParam<MaterialPropertyName>("kappa_name"))),
    _kappa_EN(declareProperty<Real>(getParam<MaterialPropertyName>("kappa_EN_name"))),
    _L(declareADProperty<Real>(getParam<MaterialPropertyName>("L_name"))),
    _L_EN(declareProperty<Real>(getParam<MaterialPropertyName>("L_EN_name"))),
    _lgb(declareADProperty<Real>(getParam<MaterialPropertyName>("lgb_name"))),
    _lgb_EN(declareProperty<Real>(getParam<MaterialPropertyName>("lgb_EN_name"))),
    _sigma(declareADProperty<Real>(getParam<MaterialPropertyName>("sigma_name"))),
    _sigmaORIUNIT(declareADProperty<Real>(getParam<MaterialPropertyName>("sigmaORIUNIT_name"))),
    _qwg(declareADProperty<Real>(getParam<MaterialPropertyName>("qwg_name"))),
    _qxg(declareADProperty<Real>(getParam<MaterialPropertyName>("qxg_name"))),
    _qyg(declareADProperty<Real>(getParam<MaterialPropertyName>("qyg_name"))),
    _qzg(declareADProperty<Real>(getParam<MaterialPropertyName>("qzg_name"))),
    _sigmaBASE(getParam<Real>("sigmaBASE")),
    _GgammaBASE(getParam<Real>("GgammaBASE")),
    _gammaBASE(getParam<Real>("gammaBASE")),
    _f0gammaBASE(getParam<Real>("f0gammaBASE")),
    _lgbBASE_minimum(getParam<Real>("lgbBASE_minimum")),
    _alphaswitch(getParam<Real>("alphaswitch")),
    _betaswitch(getParam<Real>("betaswitch")),
    _libnum(getParam<int>("libnum")),
    _amplitudeScale(getParam<Real>("amplitudeScale")),
    _sharpness(getParam<Real>("sharpness")),
    _Gaussian_Tolerance(getParam<Real>("Gaussian_Tolerance")),
    _Library_file_name(getParam<FileName>("Library_file_name")),
    _Quaternion_file_name(getParam<FileName>("Quaternion_file_name")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _length_scale(getParam<Real>("length_scale")),
    _time_scale(getParam<Real>("time_scale")),
    _JtoeV(getParam<Real>("JtoeV")),
    _GBMobility(getParam<Real>("GBMobility")),
    _Mob(getParam<Real>("Mob")),
    _Q(getParam<Real>("Q")),
    _T(getParam<Real>("T")),
    _kb(getParam<Real>("kb")),
    _BoundaryNormal(getParam<Real>("BoundaryNormal")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v")),
    _grad_vals(adCoupledGradients("v"))
{
  if (_op_num == 0)
    paramError("op_num", "op_num must be greater than 0");

  if (_GBMobility == 1 && _Mob == 0 && _Q == 0)
    mooseError("Enter GBMobility or Mob and Q");

  // Read data files during initialization
  readQuaternionFile();
  readLibraryFile();
}

void
GammaAnisoGauss::readQuaternionFile()
{
  std::ifstream File1(_Quaternion_file_name.c_str());
  if (!File1.is_open())
    mooseError("Unable to open quaternion file");

  double QuatW, QuatX, QuatY, QuatZ;
  while (File1 >> QuatW >> QuatX >> QuatY >> QuatZ)
  {
    _qwR.push_back(QuatW);
    _qxR.push_back(QuatX);
    _qyR.push_back(QuatY);
    _qzR.push_back(QuatZ);
  }

  File1.close();
}

void
GammaAnisoGauss::readLibraryFile()
{
  std::ifstream File2(_Library_file_name.c_str());
  if (!File2.is_open())
    mooseError("Unable to open library file");

  double Lib1, Lib2, Lib3, Lib4, Lib5, Lib6, Lib7, Lib8;
  while (File2 >> Lib1 >> Lib2 >> Lib3 >> Lib4 >> Lib5 >> Lib6 >> Lib7 >> Lib8)
  {
    _vbalibx.push_back(Lib1);
    _vbaliby.push_back(Lib2);
    _vbalibz.push_back(Lib3);
    _thetabalib.push_back(Lib4);
    _miubalibx.push_back(Lib5);
    _miubaliby.push_back(Lib6);
    _miubalibz.push_back(Lib7);
    _MinimaEnergy.push_back(Lib8);
  }

  File2.close();
}

void
GammaAnisoGauss::computeQpProperties()
{
  _grain_num = _grain_tracker.getTotalFeatureCount();
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (unsigned int h : make_range(0U, _grain_num))
    for (unsigned int s : make_range(0U, _op_num))
      if (h == op_to_grains[s])
      {
        _qwg[_qp] = _qwR[h];
        _qxg[_qp] = _qxR[h];
        _qyg[_qp] = _qyR[h];
        _qzg[_qp] = _qzR[h];
      }

  ADReal sumGgamma = 0.0, sumGgammax = 0.0, sumGgammay = 0.0, sumGgammaz = 0.0;
  ADReal sumGgammaxplus = 0.0, sumGgammayplus = 0.0, sumGgammazplus = 0.0;
  ADReal aniso = 0.0, anisox = 0.0, anisoy = 0.0, anisoz = 0.0;
  ADReal anisoxplus = 0.0, anisoyplus = 0.0, anisozplus = 0.0;
  ADReal Val = 0.0, Val1 = 0.0, sumval = 0.0;

  const ADReal sigmaBASE = _sigmaBASE * _JtoeV * _length_scale * _length_scale;

  _m[_qp] = std::sqrt((sigmaBASE * sigmaBASE) / ((_lgbBASE_minimum * _lgbBASE_minimum) *
                                                 _f0gammaBASE * (_GgammaBASE * _GgammaBASE)));
  _m_EN[_qp] = MetaPhysicL::raw_value(
      std::sqrt((sigmaBASE * sigmaBASE) / ((_lgbBASE_minimum * _lgbBASE_minimum) * _f0gammaBASE *
                                           (_GgammaBASE * _GgammaBASE))));

  _kappa[_qp] = (sigmaBASE * sigmaBASE) / ((_GgammaBASE * _GgammaBASE) * _m[_qp]);
  _kappa_EN[_qp] =
      MetaPhysicL::raw_value((sigmaBASE * sigmaBASE) / ((_GgammaBASE * _GgammaBASE) * _m[_qp]));

  for (unsigned int m : make_range(0U, _op_num - 1))
    for (unsigned int n : make_range(m + 1, _op_num))
    {
      Val1 = ((*_vals[m])[_qp] * (*_vals[m])[_qp]) * ((*_vals[n])[_qp] * (*_vals[n])[_qp]);
      sumval += Val1;
    }

  for (unsigned int b : make_range(0U, _grain_num - 1))
    for (unsigned int m : make_range(0U, _op_num - 1))
      for (unsigned int a : make_range(b + 1, _grain_num))
        for (unsigned int n : make_range(m + 1, _op_num))
          if (b == op_to_grains[m])
            if (a == op_to_grains[n])
            {
              aniso = _GgammaBASE;
              anisox = 0.0;
              anisoy = 0.0;
              anisoz = 0.0;
              anisoxplus = 0.0;
              anisoyplus = 0.0;
              anisozplus = 0.0;

              Val = ((*_vals[m])[_qp] * (*_vals[m])[_qp]) * ((*_vals[n])[_qp] * (*_vals[n])[_qp]);
              ADReal vsmalbax = (*_grad_vals[m])[_qp](0) - (*_grad_vals[n])[_qp](0);
              ADReal vsmalbay = (*_grad_vals[m])[_qp](1) - (*_grad_vals[n])[_qp](1);
              ADReal vsmalbaz = (*_grad_vals[m])[_qp](2) - (*_grad_vals[n])[_qp](2);
              const ADReal normvsmallbavalue =
                  vsmalbax * vsmalbax + vsmalbay * vsmalbay + vsmalbaz * vsmalbaz;
              const ADReal normvsmallba = std::sqrt(normvsmallbavalue);
              const ADReal vsmallbax = vsmalbax / normvsmallba;
              const ADReal vsmallbay = vsmalbay / normvsmallba;
              const ADReal vsmallbaz = vsmalbaz / normvsmallba;

              _Vec[_qp](0) = vsmallbax;
              _Vec[_qp](1) = vsmallbay;
              _Vec[_qp](2) = vsmallbaz;
              if (sumval == 0.0)
              {
                _Vec[_qp](0) = 0.0;
                _Vec[_qp](1) = 0.0;
                _Vec[_qp](2) = 0.0;
              }

              const ADReal qbaw = (_qwR[a] * _qwR[b]) - (_qxR[a] * -1.0 * _qxR[b]) -
                                  (_qyR[a] * -1.0 * _qyR[b]) - (_qzR[a] * -1.0 * _qzR[b]);
              const ADReal qbax = (_qwR[a] * _qxR[b]) + (_qxR[a] * -1.0 * _qwR[b]) +
                                  (_qyR[a] * -1.0 * _qzR[b]) - (_qzR[a] * -1.0 * _qyR[b]);
              const ADReal qbay = (_qwR[a] * _qyR[b]) - (_qxR[a] * -1.0 * _qzR[b]) +
                                  (_qyR[a] * -1.0 * _qwR[b]) + (_qzR[a] * -1.0 * _qxR[b]);
              const ADReal qbaz = (_qwR[a] * _qzR[b]) + (_qxR[a] * -1.0 * _qyR[b]) -
                                  (_qyR[a] * -1.0 * _qxR[b]) + (_qzR[a] * -1.0 * _qwR[b]);
              const ADReal thetaba = 2.0 * std::acos(qbaw);
              const ADReal vbaxori = qbax / std::sin(thetaba / 2.0);
              const ADReal vbayori = qbay / std::sin(thetaba / 2.0);
              const ADReal vbazori = qbaz / std::sin(thetaba / 2.0);
              const ADReal rrrvalue = vbaxori * vbaxori + vbayori * vbayori + vbazori * vbazori;
              const ADReal rrr = std::sqrt(rrrvalue);
              const ADReal vbax = vbaxori / rrr;
              const ADReal vbay = vbayori / rrr;
              const ADReal vbaz = vbazori / rrr;

              for (unsigned int l : make_range(0U, _libnum))
              {
                const ADReal miuvalue = _miubalibx[l] * _miubalibx[l] +
                                        _miubaliby[l] * _miubaliby[l] +
                                        _miubalibz[l] * _miubalibz[l];
                const ADReal mmm = std::sqrt(miuvalue);
                const ADReal miubalibx_norm = _miubalibx[l] / mmm;
                const ADReal miubaliby_norm = _miubaliby[l] / mmm;
                const ADReal miubalibz_norm = _miubalibz[l] / mmm;
                const ADReal miuow = (_qwR[a] * 0.0) - (_qxR[a] * miubalibx_norm) -
                                     (_qyR[a] * miubaliby_norm) - (_qzR[a] * miubalibz_norm);
                const ADReal miuox = (_qwR[a] * miubalibx_norm) + (_qxR[a] * 0.0) +
                                     (_qyR[a] * miubalibz_norm) - (_qzR[a] * miubaliby_norm);
                const ADReal miuoy = (_qwR[a] * miubaliby_norm) - (_qxR[a] * miubalibz_norm) +
                                     (_qyR[a] * 0.0) + (_qzR[a] * miubalibx_norm);
                const ADReal miuoz = (_qwR[a] * miubalibz_norm) + (_qxR[a] * miubaliby_norm) -
                                     (_qyR[a] * miubalibx_norm) + (_qzR[a] * 0.0);
                ADReal miubalibwR = 0.0;
                ADReal miubalibxR = miubalibx_norm;
                ADReal miubalibyR = miubaliby_norm;
                ADReal miubalibzR = miubalibz_norm;
                if (_BoundaryNormal == 0)
                {
                  miubalibwR = miuow * _qwR[a] - miuox * _qxR[a] * -1.0 - miuoy * _qyR[a] * -1.0 -
                               miuoz * _qzR[a] * -1.0;
                  miubalibxR = miuow * _qxR[a] * -1.0 + miuox * _qwR[a] + miuoy * _qzR[a] * -1.0 -
                               miuoz * _qyR[a] * -1.0;
                  miubalibyR = miuow * _qyR[a] * -1.0 - miuox * _qzR[a] * -1.0 + miuoy * _qwR[a] +
                               miuoz * _qxR[a] * -1.0;
                  miubalibzR = miuow * _qzR[a] * -1.0 + miuox * _qyR[a] * -1.0 -
                               miuoy * _qxR[a] * -1.0 + miuoz * _qwR[a];
                }
                _VlibR[_qp](0) = miubalibxR;
                _VlibR[_qp](1) = miubalibyR;
                _VlibR[_qp](2) = miubalibzR;
                if (sumval == 0.0)
                {
                  _VlibR[_qp](0) = 0.0;
                  _VlibR[_qp](1) = 0.0;
                  _VlibR[_qp](2) = 0.0;
                  miubalibwR = 0.0;
                  miubalibxR = 0.0;
                  miubalibyR = 0.0;
                  miubalibzR = 0.0;
                }

                const ADReal dot_product =
                    miubalibxR * vsmallbax + miubalibyR * vsmallbay + miubalibzR * vsmallbaz;
                if (_thetabalib[l] < 0.0)
                  _thetabalib[l] =
                      6.28319 +
                      _thetabalib[l]; // _thetabalib is in radians. 6.28319 radians is equivalent to
                                      // 360 degrees. If the angle is negative, this converts it to
                                      // its equivalent positive angle. -60deg around <111> =
                                      // (360-60 = 300deg) around <111>.
                const ADReal thetadtheta = thetaba - _thetabalib[l];
                const ADReal normvbavalue = vbax * vbax + vbay * vbay + vbaz * vbaz;
                const ADReal normvba = std::sqrt(normvbavalue);
                const ADReal normvbalibvalue = _vbalibx[l] * _vbalibx[l] +
                                               _vbaliby[l] * _vbaliby[l] +
                                               _vbalibz[l] * _vbalibz[l];
                const ADReal normvbalib = std::sqrt(normvbalibvalue);
                const ADReal cosinevalue1 =
                    _vbalibx[l] * vbax + _vbaliby[l] * vbay + _vbalibz[l] * vbaz;
                const ADReal cosinevalue2 = normvbalib * normvba;
                const ADReal cosinevalue3 = cosinevalue1 / cosinevalue2;
                const ADReal thetadv = std::acos(cosinevalue3);
                const ADReal switchvalue =
                    -100.0 * ((thetadv / _alphaswitch) * (thetadv / _alphaswitch) +
                              (thetadtheta / _betaswitch) * (thetadtheta / _betaswitch));
                ADReal exp_switch_value = std::exp(switchvalue);
                _S_switch[_qp] = exp_switch_value;
                if (exp_switch_value < 0.001) // 0.001 is an arbritary threshold below which the
                                              // switch is assumed negligible.
                  _S_switch[_qp] = 0.0;
                const ADReal Ggamma_Minima =
                    (_MinimaEnergy[l] * _JtoeV * _length_scale * _length_scale) /
                    (std::sqrt(_kappa[_qp] * _m[_qp]));
                const ADReal amplitudes = _GgammaBASE - Ggamma_Minima;
                const ADReal exponent = _sharpness * (dot_product - 1.0);
                const ADReal tol = _Gaussian_Tolerance;

                if (std::fabs(normvsmallba) > tol)
                {
                  const ADReal S_switch_value = _S_switch[_qp];
                  const ADReal exp_value = std::exp(exponent);

                  const ADReal gaussian_value = _amplitudeScale * amplitudes;
                  const ADReal Gaussian = gaussian_value * S_switch_value * exp_value;
                  const ADReal finalGaussian = _ADDGaussian ? Gaussian : -Gaussian;
                  const ADReal Gaussian2 =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((-1.0 * miubalibxR * normvsmallba) -
                        (miubalibxR * vsmalbax * (1.0 / (2.0 * normvsmallba)) *
                         (-1.0 * 2.0 * vsmalbax))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibyR * vsmalbay) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbax))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibzR * vsmalbaz) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbax))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian2 = _ADDGaussian ? Gaussian2 : -Gaussian2;
                  const ADReal Gaussian3 =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((-1.0 * miubalibyR * normvsmallba) -
                        (miubalibyR * vsmalbay * (1.0 / (2.0 * normvsmallba)) *
                         (-1.0 * 2.0 * vsmalbay))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibxR * vsmalbax) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbay))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibzR * vsmalbaz) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbay))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian3 = _ADDGaussian ? Gaussian3 : -Gaussian3;
                  const ADReal Gaussian4 =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((-1.0 * miubalibzR * normvsmallba) -
                        (miubalibzR * vsmalbaz * (1.0 / (2.0 * normvsmallba)) *
                         (-1.0 * 2.0 * vsmalbaz))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibxR * vsmalbax) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbaz))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibyR * vsmalbay) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * -1.0 * vsmalbaz))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian4 = _ADDGaussian ? Gaussian4 : -Gaussian4;
                  const ADReal Gaussian2plus =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((1.0 * miubalibxR * normvsmallba) -
                        (miubalibxR * vsmalbax * (1.0 / (2.0 * normvsmallba)) *
                         (1.0 * 2.0 * vsmalbax))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibyR * vsmalbay) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbax))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibzR * vsmalbaz) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbax))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian2plus = _ADDGaussian ? Gaussian2plus : -Gaussian2plus;
                  const ADReal Gaussian3plus =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((1.0 * miubalibyR * normvsmallba) -
                        (miubalibyR * vsmalbay * (1.0 / (2.0 * normvsmallba)) *
                         (1.0 * 2.0 * vsmalbay))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibxR * vsmalbax) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbay))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibzR * vsmalbaz) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbay))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian3plus = _ADDGaussian ? Gaussian3plus : -Gaussian3plus;
                  const ADReal Gaussian4plus =
                      _amplitudeScale * amplitudes * S_switch_value * _sharpness *
                      std::exp(exponent) *
                      (((1.0 * miubalibzR * normvsmallba) -
                        (miubalibzR * vsmalbaz * (1.0 / (2.0 * normvsmallba)) *
                         (1.0 * 2.0 * vsmalbaz))) /
                           normvsmallbavalue +
                       (((-1.0 * miubalibxR * vsmalbax) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbax))) /
                        normvsmallbavalue) +
                       (((-1.0 * miubalibyR * vsmalbay) *
                         ((1.0 / (2.0 * normvsmallba)) * (2.0 * 1.0 * vsmalbax))) /
                        normvsmallbavalue));
                  const ADReal finalGaussian4plus = _ADDGaussian ? Gaussian4plus : -Gaussian4plus;

                  aniso += finalGaussian;
                  anisox += finalGaussian2;
                  anisoy += finalGaussian3;
                  anisoz += finalGaussian4;
                  anisoxplus += finalGaussian2plus;
                  anisoyplus += finalGaussian3plus;
                  anisozplus += finalGaussian4plus;
                }
              }
              sumGgamma += aniso * Val;
              sumGgammax += anisox * Val;
              sumGgammay += anisoy * Val;
              sumGgammaz += anisoz * Val;
              sumGgammaxplus += anisoxplus * Val;
              sumGgammayplus += anisoyplus * Val;
              sumGgammazplus += anisozplus * Val;
            }

  _Ggamma[_qp] = std::fabs(sumGgamma / sumval);
  _dGgamma_minus[_qp] = {sumGgammax / sumval, sumGgammay / sumval, sumGgammaz / sumval};
  _dGgamma_plus[_qp] = {sumGgammaxplus / sumval, sumGgammayplus / sumval, sumGgammazplus / sumval};

  if (sumval == 0.0)
  {
    _Ggamma[_qp] = _GgammaBASE;
    _dGgamma_minus[_qp] = {0.0, 0.0, 0.0};
    _dGgamma_plus[_qp] = {0.0, 0.0, 0.0};
  }
  else if (_Ggamma[_qp] == 0.0)
  {
    _Ggamma[_qp] = _GgammaBASE;
    _dGgamma_minus[_qp] = {0.0, 0.0, 0.0};
    _dGgamma_plus[_qp] = {0.0, 0.0, 0.0};
  }

  const ADReal g2 = _Ggamma[_qp] * _Ggamma[_qp];
  const ADReal p = -(3.0944 * (g2 * g2 * g2 * g2)) - (1.8169 * (g2 * g2 * g2)) +
                   (10.323 * (g2 * g2)) - (8.1819 * (g2)) + 2.0033;
  const ADReal dpdg2 = -((3.0944 * 4.0) * (g2 * g2 * g2)) - ((1.8169 * 3.0) * (g2 * g2)) +
                       ((10.323 * 2.0) * (g2)) - 8.1819;

  _gamma[_qp] = 1.0 / p;
  _gamma_EN[_qp] = MetaPhysicL::raw_value(1 / p);

  const ADReal gamma = _gamma[_qp];
  _dgamma_minus[_qp] =
      (-2.0) * (_Ggamma[_qp]) * (_gamma[_qp] * _gamma[_qp]) * (dpdg2)*_dGgamma_minus[_qp];
  _dgamma_plus[_qp] =
      (-2.0) * (_Ggamma[_qp]) * (_gamma[_qp] * _gamma[_qp]) * (dpdg2)*_dGgamma_plus[_qp];

  if (sumval == 0.0)
  {
    _gamma[_qp] = _gammaBASE;
    _gamma_EN[_qp] = MetaPhysicL::raw_value(_gammaBASE);
    _dgamma_minus[_qp] = {0.0, 0.0, 0.0};
    _dgamma_plus[_qp] = {0.0, 0.0, 0.0};
  }
  else if (_Ggamma[_qp] == 0.0)
  {
    _gamma[_qp] = _gammaBASE;
    _gamma_EN[_qp] = MetaPhysicL::raw_value(_gammaBASE);
    _dgamma_minus[_qp] = {0.0, 0.0, 0.0};
    _dgamma_plus[_qp] = {0.0, 0.0, 0.0};
  }

  _sigma[_qp] = (_Ggamma[_qp]) * (std::sqrt(_kappa[_qp] * _m[_qp]));
  _sigmaORIUNIT[_qp] = (_sigma[_qp]) / (_JtoeV * _length_scale * _length_scale);

  const ADReal Inversegamma = 1.0 / (_gamma[_qp]);
  const ADReal Inversegamma2 = Inversegamma * Inversegamma;
  const ADReal Inversegamma3 = Inversegamma * Inversegamma * Inversegamma;
  const ADReal Inversegamma4 = Inversegamma * Inversegamma * Inversegamma * Inversegamma;
  const ADReal Inversegamma5 =
      Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma;
  const ADReal Inversegamma6 =
      Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma;
  const ADReal f0gamma = (0.0788 * Inversegamma6) - (0.4955 * Inversegamma5) +
                         (1.2244 * Inversegamma4) - (1.5281 * Inversegamma3) +
                         (1.0686 * Inversegamma2) - (0.5563 * Inversegamma) + 0.2907;

  _lgb[_qp] = std::sqrt(_kappa[_qp] / (_m[_qp] * f0gamma));
  _lgb_EN[_qp] = MetaPhysicL::raw_value((std::sqrt(_kappa[_qp] / (_m[_qp] * f0gamma))));

  // Calculations for isotropic L
  ADReal length_scale4 = _length_scale * _length_scale * _length_scale * _length_scale;
  ADReal Mgb = 0;
  if (_Q > 0)
  {
    ADReal M0gb = _Mob * _time_scale / (_JtoeV * length_scale4);
    Mgb = M0gb * std::exp(-_Q / (_kb * _T));
  }
  else
  {
    Mgb = _GBMobility * _time_scale / (_JtoeV * length_scale4);
  }

  _L[_qp] = (4.0 / 3.0) * (Mgb / _lgbBASE_minimum);
  _L_EN[_qp] = MetaPhysicL::raw_value((4.0 / 3.0) * (Mgb / _lgbBASE_minimum));
}
