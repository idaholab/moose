#include "TensorMechanicsPlasticWeakPlaneShear.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShear>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredRangeCheckedParam<Real>("cohesion", "cohesion>=0", "Weak plane cohesion");
  params.addRequiredRangeCheckedParam<Real>("friction_angle", "friction_angle>=0 & friction_angle<=45", "Weak-plane friction angle in degrees");
  params.addRequiredRangeCheckedParam<Real>("dilation_angle", "dilation_angle>=0", "Weak-plane dilation angle in degrees.  For associative flow use dilation_angle = friction_angle.  Should not be less than friction angle.");
  params.addParam<Real>("cohesion_residual", "Weak plane cohesion at infinite hardening.  If not given, this defaults to cohesion, ie, perfect plasticity");
  params.addParam<Real>("friction_angle_residual", "Weak-plane friction angle in degrees at infinite hardening.  If not given, this defaults to friction_angle, ie, perfect plasticity");
  params.addParam<Real>("dilation_angle_residual", "Weak-plane dilation angle in degrees at infinite hardening.  If not given, this defaults to dilation_angle, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("cohesion_rate", 0, "cohesion_rate>=0", "Cohesion = cohesion_residual + (cohesion - cohesion_residual)*exp(-cohesion_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("friction_angle_rate", 0, "friction_angle_rate>=0", "tan(friction_angle) = tan(friction_angle_residual) + (tan(friction_angle) - tan(friction_angle_residual))*exp(-friction_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("dilation_angle_rate", 0, "dilation_angle_rate>=0", "tan(dilation_angle) = tan(dilation_angle_residual) + (tan(dilation_angle) - tan(dilation_angle_residual))*exp(-dilation_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  MooseEnum tip_scheme("hyperbolic cap", "hyperbolic");
  params.addParam<MooseEnum>("tip_scheme", tip_scheme, "Scheme by which the cone's tip will be smoothed.");
  params.addRequiredRangeCheckedParam<Real>("smoother", "smoother>=0", "For the 'hyperbolic' tip_scheme, the cone vertex at shear-stress = 0 will be smoothed by the given amount.  For the 'cap' tip_scheme, additional smoothing will occur.  Typical value is 0.1*cohesion");
  params.addParam<Real>("cap_start", 0.0, "For the 'cap' tip_scheme, smoothing is performed in the stress_zz > cap_start region");
  params.addRangeCheckedParam<Real>("cap_rate", 0.0, "cap_rate>=0", "For the 'cap' tip_scheme, this controls how quickly the cap degenerates to a hemisphere: small values mean a slow degeneration to a hemisphere (and zero means the 'cap' will be totally inactive).  Typical value is 1/cohesion");
  params.addClassDescription("Non-associative finite-strain weak-plane shear plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticWeakPlaneShear::TensorMechanicsPlasticWeakPlaneShear(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _cohesion(getParam<Real>("cohesion")),
    _tan_phi(std::tan(getParam<Real>("friction_angle")*M_PI/180.0)),
    _tan_psi(std::tan(getParam<Real>("dilation_angle")*M_PI/180.0)),
    _cohesion_residual(parameters.isParamValid("cohesion_residual") ? getParam<Real>("cohesion_residual") : _cohesion),
    _tan_phi_residual(parameters.isParamValid("friction_angle_residual") ? std::tan(getParam<Real>("friction_angle_residual")*M_PI/180.0) : _tan_phi),
    _tan_psi_residual(parameters.isParamValid("dilation_angle_residual") ? std::tan(getParam<Real>("dilation_angle_residual")*M_PI/180.0) : _tan_psi),
    _cohesion_rate(getParam<Real>("cohesion_rate")),
    _tan_phi_rate(getParam<Real>("friction_angle_rate")),
    _tan_psi_rate(getParam<Real>("dilation_angle_rate")),
    _tip_scheme(getParam<MooseEnum>("tip_scheme")),
    _small_smoother2(std::pow(getParam<Real>("smoother"), 2)),
    _cap_start(getParam<Real>("cap_start")),
    _cap_rate(getParam<Real>("cap_rate"))
{
  if (_tan_phi < _tan_psi)
    mooseError("Weak-plane friction angle must not be less than weak-plane dilation angle");
  if (_cohesion_residual < 0)
    mooseError("Weak-plane residual cohesion must not be negative");
  if (_tan_phi_residual < 0 || _tan_phi_residual > 1 || _tan_psi_residual < 0 || _tan_phi_residual < _tan_psi_residual)
    mooseError("Weak-plane residual friction and dilation angles must lie in [0, 45], and dilation_residual <= friction_residual");
}


Real
TensorMechanicsPlasticWeakPlaneShear::yieldFunction(const RankTwoTensor & stress, const Real & intnl) const
{
  // note that i explicitly symmeterise in preparation for Cosserat
  return std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + smooth(stress)) + stress(2,2)*tan_phi(intnl) - cohesion(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::df_dsig(const RankTwoTensor & stress, const Real & _tan_phi_or_psi) const
{
  RankTwoTensor deriv; // the constructor zeroes this

  Real tau = std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + smooth(stress));
  // note that i explicitly symmeterise in preparation for Cosserat
  if (tau == 0.0)
  {
    // the derivative is not defined here, but i want to set it nonzero
    // because otherwise the return direction might be too crazy
    deriv(0, 2) = deriv(2, 0) = 0.5;
    deriv(1, 2) = deriv(2, 1) = 0.5;
  }
  else
  {
    deriv(0, 2) = deriv(2, 0) = 0.25*(stress(0, 2)+stress(2,0))/tau;
    deriv(1, 2) = deriv(2, 1) = 0.25*(stress(1, 2)+stress(2,1))/tau;
    deriv(2, 2) = 0.5*dsmooth(stress)/tau;
  }
  deriv(2, 2) += _tan_phi_or_psi;
  return deriv;
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const
{
  return df_dsig(stress, tan_phi(intnl));
}


Real
TensorMechanicsPlasticWeakPlaneShear::dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const
{
  return stress(2,2)*dtan_phi(intnl) - dcohesion(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::flowPotential(const RankTwoTensor & stress, const Real & intnl) const
{
  return df_dsig(stress, tan_psi(intnl));
}

RankFourTensor
TensorMechanicsPlasticWeakPlaneShear::dflowPotential_dstress(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  RankFourTensor dr_dstress;
  Real tau = std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + smooth(stress));
  if (tau == 0.0)
    return dr_dstress;

  // note that i explicitly symmeterise
  RankTwoTensor dtau;
  dtau(0, 2) = dtau(2, 0) = 0.25*(stress(0, 2) + stress(2, 0))/tau;
  dtau(1, 2) = dtau(2, 1) = 0.25*(stress(1, 2) + stress(2, 1))/tau;
  dtau(2, 2) = 0.5*dsmooth(stress)/tau;

  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned k = 0 ; k < 3 ; ++k)
        for (unsigned l = 0 ; l < 3 ; ++l)
          dr_dstress(i, j, k, l) = -dtau(i, j)*dtau(k, l)/tau;

  // note that i explicitly symmeterise
  dr_dstress(0, 2, 0, 2) += 0.25/tau;
  dr_dstress(0, 2, 2, 0) += 0.25/tau;
  dr_dstress(2, 0, 0, 2) += 0.25/tau;
  dr_dstress(2, 0, 2, 0) += 0.25/tau;
  dr_dstress(1, 2, 1, 2) += 0.25/tau;
  dr_dstress(1, 2, 2, 1) += 0.25/tau;
  dr_dstress(2, 1, 1, 2) += 0.25/tau;
  dr_dstress(2, 1, 2, 1) += 0.25/tau;
  dr_dstress(2, 2, 2, 2) += 0.5*d2smooth(stress)/tau;

  return dr_dstress;
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  RankTwoTensor dr_dintnl;
  dr_dintnl(2, 2) = dtan_psi(intnl);
  return dr_dintnl;
}

Real
TensorMechanicsPlasticWeakPlaneShear::cohesion(const Real internal_param) const
{
  return _cohesion_residual + (_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dcohesion(const Real internal_param) const
{
  return -_cohesion_rate*(_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::tan_phi(const Real internal_param) const
{
  return _tan_phi_residual + (_tan_phi - _tan_phi_residual)*std::exp(-_tan_phi_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dtan_phi(const Real internal_param) const
{
  return -_tan_phi_rate*(_tan_phi - _tan_phi_residual)*std::exp(-_tan_phi_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::tan_psi(const Real internal_param) const
{
  return _tan_psi_residual + (_tan_psi - _tan_psi_residual)*std::exp(-_tan_psi_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dtan_psi(const Real internal_param) const
{
  return -_tan_psi_rate*(_tan_psi - _tan_psi_residual)*std::exp(-_tan_psi_rate*internal_param);
}


Real
TensorMechanicsPlasticWeakPlaneShear::smooth(const RankTwoTensor & stress) const
{
  Real smoother2 = _small_smoother2;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    if (x > 0)
      p = x*(1 - std::exp(-_cap_rate*x));
    smoother2 += std::pow(p, 2);
  }
  return smoother2;
}


Real
TensorMechanicsPlasticWeakPlaneShear::dsmooth(const RankTwoTensor & stress) const
{
  Real dsmoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    if (x > 0)
    {
      p = x*(1 - std::exp(-_cap_rate*x));
      dp_dx = (1 - std::exp(-_cap_rate*x)) + x*_cap_rate*std::exp(-_cap_rate*x);
    }
    dsmoother2 += 2*p*dp_dx;
  }
  return dsmoother2;
}

Real
TensorMechanicsPlasticWeakPlaneShear::d2smooth(const RankTwoTensor & stress) const
{
  Real d2smoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    Real d2p_dx2 = 0;
    if (x > 0)
    {
      p = x*(1 - std::exp(-_cap_rate*x));
      dp_dx = (1 - std::exp(-_cap_rate*x)) + x*_cap_rate*std::exp(-_cap_rate*x);
      d2p_dx2 = 2*_cap_rate*std::exp(-_cap_rate*x) - x*std::pow(_cap_rate, 2)*std::exp(-_cap_rate*x);
    }
    d2smoother2 += 2*std::pow(dp_dx, 2) + 2*p*d2p_dx2;
  }
  return d2smoother2;
}
