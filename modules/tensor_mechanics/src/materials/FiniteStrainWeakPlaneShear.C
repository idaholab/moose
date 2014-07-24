#include "FiniteStrainWeakPlaneShear.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<FiniteStrainWeakPlaneShear>()
{
  InputParameters params = validParams<FiniteStrainPlasticBase>();

  params.addRequiredRangeCheckedParam<Real>("wps_cohesion", "wps_cohesion>=0", "Weak plane cohesion");
  params.addRequiredRangeCheckedParam<Real>("wps_friction_angle", "wps_friction_angle>=0 & wps_friction_angle<=45", "Weak-plane friction angle in degrees");
  params.addRequiredRangeCheckedParam<Real>("wps_dilation_angle", "wps_dilation_angle>=0", "Weak-plane dilation angle in degrees.  For associative flow use dilation_angle = friction_angle.  Should not be less than friction angle.");
  params.addParam<Real>("wps_cohesion_residual", "Weak plane cohesion at infinite hardening.  If not given, this defaults to wps_cohesion, ie, perfect plasticity");
  params.addParam<Real>("wps_friction_angle_residual", "Weak-plane friction angle in degrees at infinite hardening.  If not given, this defaults to wps_friction_angle, ie, perfect plasticity");
  params.addParam<Real>("wps_dilation_angle_residual", "Weak-plane dilation angle in degrees at infinite hardening.  If not given, this defaults to wps_dilation_angle, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("wps_cohesion_rate", 0, "wps_cohesion_rate>=0", "Cohesion = wps_cohesion_residual + (wps_cohesion - wps_cohesion_residual)*exp(-wps_cohesion_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("wps_friction_angle_rate", 0, "wps_friction_angle_rate>=0", "tan(friction_angle) = tan(wps_friction_angle_residual) + (tan(wps_friction_angle) - tan(wps_friction_angle_residual))*exp(-wps_friction_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("wps_dilation_angle_rate", 0, "wps_dilation_angle_rate>=0", "tan(dilation_angle) = tan(wps_dilation_angle_residual) + (tan(wps_dilation_angle) - tan(wps_dilation_angle_residual))*exp(-wps_dilation_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRequiredParam<RealVectorValue>("wps_normal_vector", "The normal vector to the weak plane");
  params.addParam<bool>("wps_normal_rotates", true, "The normal vector to the weak plane rotates with the large deformations");
  params.addRequiredRangeCheckedParam<Real>("wps_smoother", "wps_smoother>=0", "Smoothing parameter: the cone vertex at shear-stress = 0 will be smoothed by the given amount.  Typical value is 0.1*wps_cohesion");
  params.addClassDescription("Non-associative finite-strain weak-plane shear plasticity with hardening/softening");

  return params;
}

FiniteStrainWeakPlaneShear::FiniteStrainWeakPlaneShear(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainPlasticBase(name, parameters),
    _cohesion(getParam<Real>("wps_cohesion")),
    _tan_phi(std::tan(getParam<Real>("wps_friction_angle")*M_PI/180.0)),
    _tan_psi(std::tan(getParam<Real>("wps_dilation_angle")*M_PI/180.0)),
    _cohesion_residual(parameters.isParamValid("wps_cohesion_residual") ? getParam<Real>("wps_cohesion_residual") : _cohesion),
    _tan_phi_residual(parameters.isParamValid("wps_friction_angle_residual") ? std::tan(getParam<Real>("wps_friction_angle_residual")*M_PI/180.0) : _tan_phi),
    _tan_psi_residual(parameters.isParamValid("wps_dilation_angle_residual") ? std::tan(getParam<Real>("wps_dilation_angle_residual")*M_PI/180.0) : _tan_psi),
    _cohesion_rate(getParam<Real>("wps_cohesion_rate")),
    _tan_phi_rate(getParam<Real>("wps_friction_angle_rate")),
    _tan_psi_rate(getParam<Real>("wps_dilation_angle_rate")),
    _input_n(getParam<RealVectorValue>("wps_normal_vector")),
    _normal_rotates(getParam<bool>("wps_normal_rotates")),
    _small_smoother(getParam<Real>("wps_smoother")),

    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
    _wps_internal(declareProperty<Real>("weak_plane_shear_internal")),
    _wps_internal_old(declarePropertyOld<Real>("weak_plane_shear_internal")),
    _yf(declareProperty<Real>("weak_plane_shear_yield_function"))
{
  if (_tan_phi < _tan_psi)
    mooseError("Weak-plane friction angle must not be less than weak-plane dilation angle");
  if (_cohesion_residual < 0)
    mooseError("Weak-plane residual cohesion must not be negative");
  if (_tan_phi_residual < 0 || _tan_phi_residual > 1 || _tan_psi_residual < 0 || _tan_phi_residual < _tan_psi_residual)
    mooseError("Weak-plane residual friction and dilation angles must lie in [0, 45], and dilation_residual <= friction_residual");
  if (_input_n.size() == 0)
    mooseError("Weak-plane normal vector must not have zero length");
  else
    _input_n /= _input_n.size();
}


void FiniteStrainWeakPlaneShear::initQpStatefulProperties()
{
  _n[_qp] = _input_n;
  _n_old[_qp] = _input_n;
  _wps_internal[_qp] = 0;
  _wps_internal_old[_qp] = 0;
  _yf[_qp] = 0.0;
  FiniteStrainPlasticBase::initQpStatefulProperties();
}

void
FiniteStrainWeakPlaneShear::preReturnMap()
{
  // this is a rotation matrix that will rotate _n to the "z" axis
  RealTensorValue rot = RotationMatrix::rotVecToZ(_n[_qp]);

  // rotate the tensors to this frame
  _elasticity_tensor[_qp].rotate(rot);
  _stress_old[_qp].rotate(rot);
  _plastic_strain_old[_qp].rotate(rot);
  _strain_increment[_qp].rotate(rot);
}

void
FiniteStrainWeakPlaneShear::postReturnMap()
{
  // this is a rotation matrix that will rotate "z" axis back to _n
  RealTensorValue rot = RotationMatrix::rotVecToZ(_n[_qp]).transpose();

  // rotate the tensors back to original frame where _n is correctly oriented
  _elasticity_tensor[_qp].rotate(rot);
  _stress_old[_qp].rotate(rot);
  _plastic_strain_old[_qp].rotate(rot);
  _strain_increment[_qp].rotate(rot);
  _stress[_qp].rotate(rot);
  _plastic_strain[_qp].rotate(rot);

  //Rotate n by _rotation_increment, if necessary
  if (_normal_rotates)
  {
    for (unsigned int i = 0 ; i < LIBMESH_DIM ; ++i)
    {
      _n[_qp](i) = 0;
      for (unsigned int j = 0 ; j < 3 ; ++j)
        _n[_qp](i) += _rotation_increment[_qp](i, j)*_n_old[_qp](j);
    }
  }

  // Record the value of the yield function
  _yf[_qp] = _f[_qp][0];

  // Record the value of the internal parameter
  _wps_internal[_qp] = _intnl[_qp][0];

}


unsigned int
FiniteStrainWeakPlaneShear::numberOfInternalParameters()
{
  return 1;
}

void
FiniteStrainWeakPlaneShear::yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f)
{
  // note that i explicitly symmeterise in preparation for Cosserat
  f.assign(1, std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + std::pow(_small_smoother, 2)) + stress(2,2)*tan_phi(intnl[0]) - cohesion(intnl[0]));
}


RankTwoTensor
FiniteStrainWeakPlaneShear::df_dsig(const RankTwoTensor & stress, const Real & _tan_phi_or_psi)
{
  RankTwoTensor deriv; // the constructor zeroes this

  Real tau = std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + std::pow(_small_smoother, 2));
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
    deriv(0, 2) = deriv(2, 0) = 0.5*stress(0, 2)/tau;
    deriv(1, 2) = deriv(2, 1) = 0.5*stress(1, 2)/tau;
  }
  deriv(2, 2) = _tan_phi_or_psi;
  return deriv;
}


void
FiniteStrainWeakPlaneShear::dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(1, df_dsig(stress, tan_phi(intnl[0])));
}


void
FiniteStrainWeakPlaneShear::dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & df_dintnl)
{
  df_dintnl.resize(1);
  df_dintnl[0].assign(1, stress(2,2)*dtan_phi(intnl[0]) - dcohesion(intnl[0]));
}


void
FiniteStrainWeakPlaneShear::flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & r)
{
  r.assign(1, df_dsig(stress, tan_psi(intnl[0])));
}


void
FiniteStrainWeakPlaneShear::dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & /*intnl*/, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.assign(1, RankFourTensor());

  Real tau = std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + std::pow(_small_smoother, 2));
  if (tau == 0.0)
    return;

  // note that i explicitly symmeterise
  RankTwoTensor dtau;
  dtau(0, 2) = dtau(2, 0) = 0.5*stress(0, 2)/tau;
  dtau(1, 2) = dtau(2, 1) = 0.5*stress(1, 2)/tau;

  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned k = 0 ; k < 3 ; ++k)
        for (unsigned l = 0 ; l < 3 ; ++l)
          dr_dstress[0](i, j, k, l) = -dtau(i, j)*dtau(k, l)/tau;

  // note that i explicitly symmeterise
  dr_dstress[0](0, 2, 0, 2) += 0.25/tau;
  dr_dstress[0](0, 2, 2, 0) += 0.25/tau;
  dr_dstress[0](2, 0, 0, 2) += 0.25/tau;
  dr_dstress[0](2, 0, 2, 0) += 0.25/tau;
  dr_dstress[0](1, 2, 1, 2) += 0.25/tau;
  dr_dstress[0](1, 2, 2, 1) += 0.25/tau;
  dr_dstress[0](2, 1, 1, 2) += 0.25/tau;
  dr_dstress[0](2, 1, 2, 1) += 0.25/tau;
}

void
FiniteStrainWeakPlaneShear::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dr_dintnl)
{
  dr_dintnl.resize(1);
  dr_dintnl[0].assign(1, RankTwoTensor());
  dr_dintnl[0][0](2, 2) = dtan_psi(intnl[0]);
}

void
FiniteStrainWeakPlaneShear::hardPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<Real> > & h)
{
  h.resize(1);
  h[0].assign(1, -1.0);
}

void
FiniteStrainWeakPlaneShear::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dh_dstress)
{
  dh_dstress.resize(1);
  dh_dstress[0].assign(1, RankTwoTensor());
}

void
FiniteStrainWeakPlaneShear::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<std::vector<Real> > > & dh_dintnl)
{
  dh_dintnl.resize(1);
  dh_dintnl[0].resize(1);
  dh_dintnl[0][0].assign(1, 0.0);
}


Real
FiniteStrainWeakPlaneShear::cohesion(const Real internal_param)
{
  return _cohesion_residual + (_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
FiniteStrainWeakPlaneShear::dcohesion(const Real internal_param)
{
  return -_cohesion_rate*(_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
FiniteStrainWeakPlaneShear::tan_phi(const Real internal_param)
{
  return _tan_phi_residual + (_tan_phi - _tan_phi_residual)*std::exp(-_tan_phi_rate*internal_param);
}

Real
FiniteStrainWeakPlaneShear::dtan_phi(const Real internal_param)
{
  return -_tan_phi_rate*(_tan_phi - _tan_phi_residual)*std::exp(-_tan_phi_rate*internal_param);
}

Real
FiniteStrainWeakPlaneShear::tan_psi(const Real internal_param)
{
  return _tan_psi_residual + (_tan_psi - _tan_psi_residual)*std::exp(-_tan_psi_rate*internal_param);
}

Real
FiniteStrainWeakPlaneShear::dtan_psi(const Real internal_param)
{
  return -_tan_psi_rate*(_tan_psi - _tan_psi_residual)*std::exp(-_tan_psi_rate*internal_param);
}
