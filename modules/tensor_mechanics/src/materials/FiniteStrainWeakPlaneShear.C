#include "FiniteStrainWeakPlaneShear.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<FiniteStrainWeakPlaneShear>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

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
  params.addRequiredRangeCheckedParam<Real>("wps_f_tol", "wps_f_tol>0", "Tolerance on the yield function: if yield function is less than this value then the stresses are admissible");
  params.addRequiredRangeCheckedParam<Real>("wps_r_tol", "wps_r_tol>0", "Tolerance on the residual function: if the L2 norm of the residual is less than this value then the Newton-Raphson procedure is deemed to have converged");
  params.addRequiredRangeCheckedParam<Real>("wps_smoother", "wps_smoother>=0", "Smoothing parameter: the cone vertex at shear-stress = 0 will be smoothed by the given amount.  Typical value is 0.1*wps_cohesion");
  params.addRangeCheckedParam<int>("wps_max_iterations", 20, "wps_max_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addClassDescription("Non-associative weak-plane shear plasticity with no hardening");

  return params;
}

FiniteStrainWeakPlaneShear::FiniteStrainWeakPlaneShear(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
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
    _f_tol(getParam<Real>("wps_f_tol")),
    _r_tol(getParam<Real>("wps_r_tol")),
    _small_smoother(getParam<Real>("wps_smoother")),
    _max_iter(getParam<int>("wps_max_iterations")),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
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
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
  _wps_internal[_qp] = 0;
  _wps_internal_old[_qp] = 0;
}

void FiniteStrainWeakPlaneShear::computeQpStress()
{
  // This is the total strain.  Pritam is re-working this part of the code (15 July 2014)
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

  // this is a rotation matrix that will rotate _n to the "z" axis
  RankTwoTensor rot(RotationMatrix::rotVecToZ(_n[_qp])); // convert from the RealTensor to expedite the rotations below

  // rotate the old values of stress and plastic strain to this frame, so yield function involves s_xz, s_yz and s_zz
  RankTwoTensor tilde_sigma_old = rot*_stress_old[_qp]*rot.transpose();
  RankTwoTensor tilde_epp_old = rot*_plastic_strain_old[_qp]*rot.transpose();
  RankTwoTensor tilde_incr = rot*_strain_increment[_qp]*rot.transpose();


  // perform the return-mapping algorithm
  RankTwoTensor tilde_sigma;
  RankTwoTensor tilde_epp;
  returnMap(tilde_sigma_old, tilde_epp_old, _wps_internal_old[_qp], tilde_incr, _elasticity_tensor[_qp], tilde_sigma, tilde_epp, _wps_internal[_qp], _yf[_qp]);


  // rotate the stress and plastic strain back to original frame where _n is correctly oriented
  _stress[_qp] = rot.transpose()*tilde_sigma*rot;
  _plastic_strain[_qp] = rot.transpose()*tilde_epp*rot;

  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();

  //Rotate to plastic rate of deformation tensor the current configuration
  _plastic_strain[_qp] = _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();

  //Rotate n, if necessary
  if (_normal_rotates)
  {
    for (unsigned int i = 0 ; i < LIBMESH_DIM ; ++i)
    {
      _n[_qp](i) = 0;
      for (unsigned int j = 0 ; j < 3 ; ++j)
        _n[_qp](i) += _rotation_increment[_qp](i, j)*_n_old[_qp](j);
    }
  }
}

void
FiniteStrainWeakPlaneShear::returnMap(const RankTwoTensor & sig_old, const RankTwoTensor &plastic_strain_old, const Real &internal_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & sig, RankTwoTensor & plastic_strain, Real &internal, Real & f)
{
  // the consistency parameter, must be non-negative
  // change in plastic strain in this timestep = flow_incr*flow_potential
  Real flow_incr = 0.0;

  // direction of flow defined by the potential
  RankTwoTensor flow_dirn;

  // Newton-Raphson sets this zero
  // resid_ij = flow_incr*flow_dirn_ij - (plastic_strain - plastic_strain_old)
  RankTwoTensor resid;

  // Newton-Raphson sets this "internal constraint" to zero
  // ic = internal - internal_old - flow_incr;
  Real ic;

  // change in the stress (sig) in a Newton-Raphson iteration
  RankTwoTensor ddsig;

  // change in the consistency parameter in a Newton-Raphson iteration
  Real dflow_incr = 0.0;

  // change in the internal parameter in a Newton-Raphson iteration
  Real dinternal = 0.0;

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(trial_stress - sig), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp;

  // d(yieldFunction)/d(stress)
  RankTwoTensor df_dsig;

  // d(yieldFunction)/d(internal)
  Real df_di;

  // d(resid_ij)/d(sigma_kl)
  RankFourTensor dr_dsig;

  // dr_dsig_inv_ijkl*dr_dsig_klmn = 0.5*(de_ij de_jn + de_ij + de_jm), where de_ij = 1 if i=j, but zero otherwise
  RankFourTensor dr_dsig_inv;

  // d(resid_ij)/d(internal)
  RankTwoTensor dr_di;

  // Newton-Raphson residual-squared
  Real nr_res2;

  // number of Newton-Raphson iterations performed
  unsigned int iter = 0;


  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  sig = sig_old + E_ijkl * delta_d;  // the trial stress
  plastic_strain = plastic_strain_old;
  internal = internal_old;

  f = yieldFunction(sig, internal);
  if (f > _f_tol)
  {
    // the sig just calculated is inadmissable.  We must return to the yield surface.
    // This is done iteratively, using a Newton-Raphson process.

    delta_dp.zero();

    sig = sig_old + E_ijkl * delta_d;  // this is the elastic predictor, which is initial-guess to NR

    flow_dirn = flowPotential(sig, internal);

    // need the following to be zero
    resid = flow_dirn * flow_incr - delta_dp;
    f = yieldFunction(sig, internal);
    ic = internal - internal_old - flow_incr;

    nr_res2 = 0.5*(std::pow(f/_f_tol, 2) + std::pow(resid.L2norm()/_r_tol, 2) + std::pow(ic/_r_tol, 2));


    while (nr_res2 > 0.5 && iter < static_cast<unsigned>(_max_iter))
    {
      iter++;

      RankFourTensor E_inv = E_ijkl.invSymm();

      df_dsig = dyieldFunction_dstress(sig, tan_phi(internal));
      df_di = dyieldFunction_dinternal(sig, internal);
      dr_dsig = dflowPotential_dstress(sig, internal)*flow_incr + E_inv;
      dr_di = dflowPotential_dinternal(sig, internal)*flow_incr;

      /**
       * The linear system is
       *   ( dr_dsig  flow_dirn dr_di)( ddsig       )   ( - resid )
       *   ( df_dsig     0      df_di)( dflow_incr  ) = ( - f     )
       *   ( 0           -1       1  )( dinternal   ) = ( - ic    )
       */

      //Moose::out << "iter = " << iter << " should be pos = " << df_dsig.doubleContraction(E_ijkl * flow_dirn) << "\n";

      dr_dsig_inv = dr_dsig.invSymm();

      /**
       * Because of the zero, the linear system is not impossible to
       * solve by hand.
       */
      dflow_incr = (f - df_dsig.doubleContraction(dr_dsig_inv * resid)) / df_dsig.doubleContraction(dr_dsig_inv * flow_dirn);
      dflow_incr = (f - df_dsig.doubleContraction(dr_dsig_inv * (resid - dr_di*ic))) / (df_dsig.doubleContraction(dr_dsig_inv * (flow_dirn + dr_di)) - df_di);
      dinternal = -ic + dflow_incr;
      ddsig = dr_dsig_inv * (-resid - flow_dirn*dflow_incr - dr_di*dinternal);  // from solving the top row of linear system, given dflow_incr and dinternal

      // perform a line search
      // Want to decrease nr_res2
      // The following algorithm comes straight from "Numerical Recipes"
      Real lam = 1.0; // the line-search parameter
      Real lam_min = 1E-7; // minimum value of lam allowed - perhaps this should be dynamically calculated?
      bool line_searching = true;
      Real f0 = nr_res2;
      Real slope = resid.doubleContraction(dr_dsig*ddsig + flow_dirn*dflow_incr + dr_di*dinternal)/std::pow(_r_tol, 2) + f*(df_dsig.doubleContraction(ddsig) + df_di*dinternal)/std::pow(_f_tol, 2) + ic*(-dflow_incr + dinternal)/std::pow(_r_tol, 2);
      if (slope > 0)
        mooseError("Roundoff problem in weak-plane line search");
      Real ls_flow_incr; // flow_incr during the line-search
      RankTwoTensor ls_delta_dp; // delta_dp during the line-search
      Real ls_internal; // internal parameter during the line-search
      RankTwoTensor ls_sig; // stress during the line-search
      Real tmp_lam; // cached value of lam used in quadratic & cubic line search
      Real f2; // cached value of f = nr_res2 used in the cubic in the line search
      Real lam2; // cached value of lam used in the cubic in the line search
      while (line_searching)
      {
        // update the variables using this line-search parameter
        ls_flow_incr = flow_incr + dflow_incr*lam;
        ls_delta_dp = delta_dp - (E_inv*ddsig)*lam;
        ls_internal = internal + dinternal*lam;
        ls_sig = sig + ddsig*lam;

        // calculate the new yield function and residual
        flow_dirn = flowPotential(ls_sig, ls_internal);
        resid = flow_dirn*ls_flow_incr - ls_delta_dp;
        f = yieldFunction(ls_sig, ls_internal);
        ic = ls_internal - internal_old - ls_flow_incr;

        nr_res2 = 0.5*(std::pow(f/_f_tol, 2) + std::pow(resid.L2norm()/_r_tol, 2) + std::pow(ic/_r_tol, 2));

        if (nr_res2 < f0 + 1E-4*lam*slope)
        {
          //if (lam < 1)
          //  Moose::out << "Iteration " << iter << " Found line search lambda = " << lam << "\n";
          line_searching = false;
          break;
        }
        else if (lam < lam_min)
        {
          _console << "WARNING: Weak Plane Shear ine search does not decrease norm of residual at NR iteration " << iter << "\n";
          lam = 0.1;
          ls_flow_incr = flow_incr + dflow_incr*lam;
          ls_delta_dp = delta_dp - (E_inv*ddsig)*lam;
          ls_internal = internal + dinternal*lam;
          ls_sig = sig + ddsig*lam;
          flow_dirn = flowPotential(ls_sig, ls_internal);
          resid = flow_dirn*ls_flow_incr - ls_delta_dp;
          f = yieldFunction(ls_sig, ls_internal);
          ic = ls_internal - internal_old - ls_flow_incr;
          nr_res2 = 0.5*(std::pow(f/_f_tol, 2) + std::pow(resid.L2norm()/_r_tol, 2) + std::pow(ic/_r_tol, 2));
          line_searching = false;
          break;
        }
        else if (lam == 1.0)
        {
          // model as a quadratic
          tmp_lam = -slope/2.0/(nr_res2 - f0 - slope);
        }
        else
        {
          // model as a cubic
          Real rhs1 = nr_res2 - f0 - lam*slope;
          Real rhs2 = f2 - f0 - lam2*slope;
          Real a = (rhs1/std::pow(lam, 2) - rhs2/std::pow(lam2, 2))/(lam - lam2);
          Real b = (-lam2*rhs1/std::pow(lam, 2) + lam*rhs2/std::pow(lam2, 2))/(lam - lam2);
          if (a == 0)
            tmp_lam = -slope/2.0/b;
          else
          {
            Real disc = std::pow(b, 2) - 3*a*slope;
            if (disc < 0)
              tmp_lam = 0.5*lam;
            else if (b <= 0)
              tmp_lam = (-b + std::sqrt(disc))/3.0/a;
            else
              tmp_lam = -slope/(b + std::sqrt(disc));
          }
          if (tmp_lam > 0.5*lam)
            tmp_lam = 0.5*lam;
        }
        lam2 = lam;
        f2 = nr_res2;
        lam = std::max(tmp_lam, 0.1*lam);
        //Moose::out << "Line search f = " << nr_res2 << " Need " << f0 + 1E-4*lam*slope << " trying lam = " << lam << "\n";
      }

      flow_incr = ls_flow_incr;
      delta_dp = ls_delta_dp;
      internal = ls_internal;
      sig = ls_sig;


      //Moose::out << "iter = " << iter << " lam " << lam << " dflow_incr = " << dflow_incr << " f = " << f << " |resid| = " << resid.L2norm() << "\n";
      //sig.print();
    }

    if (iter >= static_cast<unsigned>(_max_iter))
    {
      sig = sig_old;
      internal = internal_old;
      _console << "Too many iterations in Weak Plane Shear.  f = " << f << ", |resid| = " << resid.L2norm() << ", condition = " << std::abs(f)/_f_tol + resid.L2norm()/_r_tol << "\n";
      return;
    }

    plastic_strain += delta_dp;
  }
}


Real
FiniteStrainWeakPlaneShear::yieldFunction(const RankTwoTensor &stress, const Real internal_param)
{
  // note that i explicitly symmeterise in preparation for Cosserat
  return std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + std::pow(_small_smoother, 2)) + stress(2,2)*tan_phi(internal_param) - cohesion(internal_param);
}


RankTwoTensor
FiniteStrainWeakPlaneShear::dyieldFunction_dstress(const RankTwoTensor & stress, const Real tan_fric_or_dil)
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
  deriv(2, 2) = tan_fric_or_dil;
  return deriv;
}


Real
FiniteStrainWeakPlaneShear::dyieldFunction_dinternal(const RankTwoTensor &stress, const Real internal_param)
{
  return stress(2,2)*dtan_phi(internal_param) - dcohesion(internal_param);
}



RankTwoTensor
FiniteStrainWeakPlaneShear::flowPotential(const RankTwoTensor & stress, const Real internal_param)
{
  return dyieldFunction_dstress(stress, tan_psi(internal_param));
}


RankFourTensor
FiniteStrainWeakPlaneShear::dflowPotential_dstress(const RankTwoTensor & stress, const Real /*internal_param*/)
{
  RankFourTensor d2tau; // the constructor zeroes this

  Real tau = std::sqrt(std::pow((stress(0,2) + stress(2,0))/2, 2) + std::pow((stress(1,2) + stress(2,1))/2, 2) + std::pow(_small_smoother, 2));
  if (tau == 0.0)
    return d2tau;

  // note that i explicitly symmeterise in preparation for Cosserat
  RankTwoTensor dtau;
  dtau(0, 2) = dtau(2, 0) = 0.5*stress(0, 2)/tau;
  dtau(1, 2) = dtau(2, 1) = 0.5*stress(1, 2)/tau;

  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned k = 0 ; k < 3 ; ++k)
        for (unsigned l = 0 ; l < 3 ; ++l)
          d2tau(i, j, k, l) = -dtau(i, j)*dtau(k, l)/tau;

  // note that i explicitly symmeterise
  d2tau(0, 2, 0, 2) += 0.25/tau;
  d2tau(0, 2, 2, 0) += 0.25/tau;
  d2tau(2, 0, 0, 2) += 0.25/tau;
  d2tau(2, 0, 2, 0) += 0.25/tau;
  d2tau(1, 2, 1, 2) += 0.25/tau;
  d2tau(1, 2, 2, 1) += 0.25/tau;
  d2tau(2, 1, 1, 2) += 0.25/tau;
  d2tau(2, 1, 2, 1) += 0.25/tau;

  return d2tau;
}

RankTwoTensor
FiniteStrainWeakPlaneShear::dflowPotential_dinternal(const RankTwoTensor & /*stress*/, const Real internal_param)
{
  RankTwoTensor deriv; // constructor zeroes this
  deriv(2, 2) = dtan_psi(internal_param);
  return deriv;
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
