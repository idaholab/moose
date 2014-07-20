#include "FiniteStrainWeakPlaneTensile.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<FiniteStrainWeakPlaneTensile>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();
  params.addRangeCheckedParam<Real>("tension_cutoff","tension_cutoff>0","Tension cutoff has to be positive");
  params.addRequiredParam<RealVectorValue>("wpt_normal_vector", "The normal vector to the weak plane");
  params.addParam<bool>("wpt_normal_rotates", true, "The normal vector to the weak plane rotates with the large deformations");
  params.addRequiredRangeCheckedParam<Real>("wpt_f_tol", "wpt_f_tol>0", "Tolerance on the yield function: if yield function is less than this value then the stresses are admissible");
  params.addRequiredRangeCheckedParam<Real>("wpt_r_tol", "wpt_r_tol>0", "Tolerance on the residual function: if the L2 norm of the residual is less than this value then the Newton-Raphson procedure is deemed to have converged");
  params.addRangeCheckedParam<int>("wpt_max_iterations", 20, "wpt_max_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addClassDescription("Non-associative weak-plane tensile plasticity with no hardening");

  return params;
}

FiniteStrainWeakPlaneTensile::FiniteStrainWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
    _tension_cutoff(getParam<Real>("tension_cutoff")),
    _input_n(getParam<RealVectorValue>("wpt_normal_vector")),
    _normal_rotates(getParam<bool>("wpt_normal_rotates")),
    _f_tol(getParam<Real>("wpt_f_tol")),
    _r_tol(getParam<Real>("wpt_r_tol")),
    //_small_smoother(getParam<Real>("wps_smoother")),
    _max_iter(getParam<int>("wpt_max_iterations")),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
    _yf(declareProperty<Real>("weak_plane_tensile_yield_function"))
    
{
   if (_input_n.size() == 0)
    mooseError("Weak-plane normal vector must not have zero length");
  else
    _input_n /= _input_n.size();
}

void FiniteStrainWeakPlaneTensile::initQpStatefulProperties()
{
  _n[_qp] = _input_n;
  _n_old[_qp] = _input_n;
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
}


void FiniteStrainWeakPlaneTensile::computeQpStress()
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
  returnMap(tilde_sigma_old, tilde_epp_old, tilde_incr, _elasticity_tensor[_qp], tilde_sigma, tilde_epp, _yf[_qp]);

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
FiniteStrainWeakPlaneTensile::returnMap(const RankTwoTensor & sig_old, const RankTwoTensor &plastic_strain_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & sig, RankTwoTensor & plastic_strain, Real & f)
{
  // the consistency parameter, must be non-negative
  // change in plastic strain in this timestep = flow_incr*flow_potential
  Real flow_incr = 0.0;

  // direction of flow defined by the potential
  RankTwoTensor flow_dirn;

  // Newton-Raphson sets this zero
  // resid_ij = flow_incr*flow_dirn_ij - (plastic_strain - plastic_strain_old)
  RankTwoTensor resid;

  // change in the stress (sig) in a Newton-Raphson iteration
  RankTwoTensor ddsig;

  // change in the consistency parameter in a Newton-Raphson iteration
  Real dflow_incr = 0.0;

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(trial_stress - sig), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp;

  // d(yieldFunction)/d(stress)
  RankTwoTensor df_dsig;

  // d(resid_ij)/d(sigma_kl)
  RankFourTensor dr_dsig;

  // dr_dsig_inv_ijkl*dr_dsig_klmn = 0.5*(de_ij de_jn + de_ij + de_jm), where de_ij = 1 if i=j, but zero otherwise
  RankFourTensor dr_dsig_inv;

  // Newton-Raphson residual-squared
  Real nr_res2;

  // number of Newton-Raphson iterations performed
  unsigned int iter = 0;


  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  sig = sig_old + E_ijkl * delta_d;  // the trial stress
  plastic_strain = plastic_strain_old;

  f = yieldFunction(sig);
  if (f > _f_tol)
  {
    // the sig just calculated is inadmissable.  We must return to the yield surface.
    // This is done iteratively, using a Newton-Raphson process.

    delta_dp.zero();

    sig = sig_old + E_ijkl * delta_d;  // this is the elastic predictor

    flow_dirn = flowPotential(sig);

    // need the following to be zero
    resid = flow_dirn * flow_incr - delta_dp;
    f = yieldFunction(sig);

    nr_res2 = 0.5*(std::pow(f/_f_tol, 2) + std::pow(resid.L2norm()/_r_tol, 2));


    while (nr_res2 > 0.5 && iter < _max_iter)
    {
      iter++;

      RankFourTensor E_inv = E_ijkl.invSymm();

      df_dsig = dyieldFunction_dstress(sig);
      dr_dsig = dflowPotential_dstress(sig)*flow_incr + E_inv;

      /**
       * The linear system is
       *   ( dr_dsig  flow_dirn )( ddsig       )   ( - resid )
       *   ( df_dsig     0      )( dflow_incr  ) = ( - f     )
       */

      //Moose::out << "iter = " << iter << " should be pos = " << df_dsig.doubleContraction(E_ijkl * flow_dirn) << "\n";
      //flow_dirn.print();

      dr_dsig_inv = dr_dsig.invSymm();

      /**
       * Because of the zero, the linear system is not impossible to
       * solve by hand.
       */
      dflow_incr = (f - df_dsig.doubleContraction(dr_dsig_inv * resid)) / df_dsig.doubleContraction(dr_dsig_inv * flow_dirn);
      ddsig = dr_dsig_inv * (-resid - flow_dirn * dflow_incr);  // from solving the top row of linear system, given dflow_incr

      flow_incr += dflow_incr;
      delta_dp += -E_inv*ddsig;
      sig += ddsig;
      
      flow_dirn = flowPotential(sig);

       // need the following to be zero
      //resid = flow_dirn * flow_incr - delta_dp;
      f = yieldFunction(sig);
      // need the following to be zero
      resid = flow_dirn * flow_incr - delta_dp;
      nr_res2 = 0.5*(std::pow(f/_f_tol, 2) + std::pow(resid.L2norm()/_r_tol, 2));

      //Moose::out << "iter = " << iter << " dflow_incr = " << dflow_incr << " f = " << f << " |resid| = " << resid.L2norm() << "\n";
      //sig.print();
    }

    if (iter >= _max_iter)
    {
      sig = sig_old;
      _console << "Too many iterations in Weak Plane Shear.  f = " << f << ", |resid| = " << resid.L2norm() << ", condition = " << std::abs(f)/_f_tol + resid.L2norm()/_r_tol << "\n";
      return;
    }

    plastic_strain += delta_dp;
  }
}


Real
FiniteStrainWeakPlaneTensile::yieldFunction(const RankTwoTensor &stress)
{
  return (stress(2,2) - _tension_cutoff);
}

RankTwoTensor
FiniteStrainWeakPlaneTensile::dyieldFunction_dstress(const RankTwoTensor & stress)
{
  RankTwoTensor deriv; // the constructor zeroes this
  
  deriv(2,2) = 1.0;
  return deriv;
}

RankTwoTensor
FiniteStrainWeakPlaneTensile::flowPotential(const RankTwoTensor & stress)
{
  return dyieldFunction_dstress(stress);
}

RankFourTensor
FiniteStrainWeakPlaneTensile::dflowPotential_dstress(const RankTwoTensor & stress)
{
  RankFourTensor d2sigma; // the constructor zeroes this
  return d2sigma;
}