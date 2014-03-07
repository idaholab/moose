/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "VG1" form of relative permeability
//
#include "RichardsRelPermVG1.h"

template<>
InputParameters validParams<RichardsRelPermVG1>()
{
  InputParameters params = validParams<RichardsRelPermVG>();
  params.addRequiredParam<Real>("simm", "Immobile saturation.  Must be between 0 and 1.  Define s = (seff - simm)/(1 - simm).  Then relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2");
  params.addRequiredParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set >0.5.  Define s = (seff - simm)/(1 - simm).  Then relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2");
  params.addRequiredParam<Real>("scut", "cutoff in effective saturation.");
  params.addClassDescription("VG1 form of relative permeability.  Define s = (seff - simm)/(1 - simm).  Then relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2, if s>0, and relperm=0 otherwise");
  return params;
}

RichardsRelPermVG1::RichardsRelPermVG1(const std::string & name, InputParameters parameters) :
  RichardsRelPermVG(name, parameters),
  _simm(getParam<Real>("simm")),
  _m(getParam<Real>("m")),
  _scut(getParam<Real>("scut")),
  _vg1_const(0),
  _vg1_linear(0),
  _vg1_quad(0),
  _vg1_cub(0)
{
  if (_simm < 0 || _simm > 1)
    mooseError("Immobile saturation set to " << _simm << " in relative permeability function but it must not be less than zero or greater than 1");
  if (_m <= 0 || _m >= 1)
    mooseError("Van Genuchten m parameter in relative permeability is set to " << _m << " but it must be between 0 and 1, and optimally >0.5");
  if (_scut <= 0 || _scut >= 1)
    mooseError("The cutoff saturation in the VG1 type of relative permeability is set to " << _scut << " but this must be between 0 and 1");

  _vg1_const = RichardsRelPermVG::relperm(_scut);
  _vg1_linear = RichardsRelPermVG::drelperm(_scut);
  _vg1_quad = RichardsRelPermVG::d2relperm(_scut);
  _vg1_cub = (1 - _vg1_const - _vg1_linear*(1 - _scut) - _vg1_quad*std::pow(1 - _scut, 2))/std::pow(1 - _scut, 3);
  Moose::out << "Relative permeability of VG1 type has cubic coefficients " << _vg1_const << " " << _vg1_linear << " " << _vg1_quad << " " << _vg1_cub << "\n";
}


Real
RichardsRelPermVG1::relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 1.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);

  if (s_internal < _scut) return RichardsRelPermVG::relperm(seff);

  Real krel = _vg1_const + _vg1_linear*(s_internal - _scut) + _vg1_quad*std::pow(s_internal - _scut, 2) + _vg1_cub*std::pow(s_internal - _scut, 3);

  // bound, just in case
  if (krel < 0) { krel = 0;}
  if (krel > 1) { krel = 1;}
  return krel;
}


Real
RichardsRelPermVG1::drelperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);

  if (s_internal < _scut) return RichardsRelPermVG::drelperm(seff);

  Real krelp = _vg1_linear + 2*_vg1_quad*(s_internal - _scut) + 3*_vg1_cub*std::pow(s_internal - _scut, 2);
  return krelp/(1.0 - _simm);
}


Real
RichardsRelPermVG1::d2relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);

  if (s_internal < _scut) return RichardsRelPermVG::d2relperm(seff);

  Real krelpp = 2*_vg1_quad + 6*_vg1_cub*(s_internal - _scut);
  return krelpp/std::pow(1.0 - _simm, 2);
}
