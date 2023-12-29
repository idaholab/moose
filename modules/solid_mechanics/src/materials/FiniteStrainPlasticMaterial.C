//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FiniteStrainPlasticMaterial.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", FiniteStrainPlasticMaterial);

InputParameters
FiniteStrainPlasticMaterial::validParams()
{
  InputParameters params = ComputeStressBase::validParams();

  params.addRequiredParam<std::vector<Real>>(
      "yield_stress",
      "Input data as pairs of equivalent plastic strain and yield stress: Should "
      "start with equivalent plastic strain 0");
  params.addParam<Real>("rtol", 1e-8, "Plastic strain NR tolerance");
  params.addParam<Real>("ftol", 1e-4, "Consistency condition NR tolerance");
  params.addParam<Real>("eptol", 1e-7, "Equivalent plastic strain NR tolerance");
  params.addClassDescription("Associative J2 plasticity with isotropic hardening.");

  return params;
}

FiniteStrainPlasticMaterial::FiniteStrainPlasticMaterial(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _yield_stress_vector(getParam<std::vector<Real>>("yield_stress")), // Read from input file
    _plastic_strain(declareProperty<RankTwoTensor>(_base_name + "plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "plastic_strain")),
    _eqv_plastic_strain(declareProperty<Real>(_base_name + "eqv_plastic_strain")),
    _eqv_plastic_strain_old(getMaterialPropertyOld<Real>(_base_name + "eqv_plastic_strain")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(getMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _rtol(getParam<Real>("rtol")),
    _ftol(getParam<Real>("ftol")),
    _eptol(getParam<Real>("eptol")),
    _deltaOuter(RankTwoTensor::Identity().times<i_, j_, k_, l_>(RankTwoTensor::Identity())),
    _deltaMixed(RankTwoTensor::Identity().times<i_, k_, j_, l_>(RankTwoTensor::Identity()))
{
}

void
FiniteStrainPlasticMaterial::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _plastic_strain[_qp].zero();
  _eqv_plastic_strain[_qp] = 0.0;
}

void
FiniteStrainPlasticMaterial::computeQpStress()
{

  // perform the return-mapping algorithm
  returnMap(_stress_old[_qp],
            _eqv_plastic_strain_old[_qp],
            _plastic_strain_old[_qp],
            _strain_increment[_qp],
            _elasticity_tensor[_qp],
            _stress[_qp],
            _eqv_plastic_strain[_qp],
            _plastic_strain[_qp]);

  // Rotate the stress tensor to the current configuration
  _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();

  // Rotate plastic strain tensor to the current configuration
  _plastic_strain[_qp] =
      _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();

  // Calculate the elastic strain_increment
  _elastic_strain[_qp] = _mechanical_strain[_qp] - _plastic_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

/**
 * Implements the return-map algorithm via a Newton-Raphson process.
 * This idea is fully explained in Simo and Hughes "Computational
 * Inelasticity" Springer 1997, for instance, as well as many other
 * books on plasticity.
 * The basic idea is as follows.
 * Given: sig_old - the stress at the start of the "time step"
 *        plastic_strain_old - the plastic strain at the start of the "time step"
 *        eqvpstrain_old - equivalent plastic strain at the start of the "time step"
 *                         (In general we would be given some number of internal
 *                         parameters that the yield function depends upon.)
 *        delta_d - the prescribed strain increment for this "time step"
 * we want to determine the following parameters at the end of this "time step":
 *        sig - the stress
 *        plastic_strain - the plastic strain
 *        eqvpstrain - the equivalent plastic strain (again, in general, we would
 *                     have an arbitrary number of internal parameters).
 *
 * To determine these parameters, introduce
 *    the "yield function", f
 *    the "consistency parameter", flow_incr
 *    the "flow potential", flow_dirn_ij
 *    the "internal potential", internalPotential (in general there are as many internalPotential
 *        functions as there are internal parameters).
 * All three of f, flow_dirn_ij, and internalPotential, are functions of
 * sig and eqvpstrain.
 * To find sig, plastic_strain and eqvpstrain, we need to solve the following
 *   resid_ij = 0
 *   f = 0
 *   rep = 0
 * This is done by using Newton-Raphson.
 * There are 8 equations here: six from resid_ij=0 (more generally there are nine
 * but in this case resid is symmetric); one from f=0; one from rep=0 (more generally, for N
 * internal parameters there are N of these equations).
 *
 * resid_ij = flow_incr*flow_dirn_ij - (plastic_strain - plastic_strain_old)_ij
 *          = flow_incr*flow_dirn_ij - (E^{-1}(trial_stress - sig))_ij
 * Here trial_stress = E*(strain - plastic_strain_old)
 *      sig = E*(strain - plastic_strain)
 * Note: flow_dirn_ij is evaluated at sig and eqvpstrain (not the old values).
 *
 * f is the yield function, evaluated at sig and eqvpstrain
 *
 * rep = -flow_incr*internalPotential - (eqvpstrain - eqvpstrain_old)
 * Here internalPotential are evaluated at sig and eqvpstrain (not the old values).
 *
 * The Newton-Raphson procedure has sig, flow_incr, and eqvpstrain as its
 * variables.  Therefore we need the derivatives of resid_ij, f, and rep
 * with respect to these parameters
 *
 * In this associative J2 with isotropic hardening, things are a little more specialised.
 * (1) f = sqrt(3*s_ij*s_ij/2) - K(eqvpstrain)    (this is called "isotropic hardening")
 * (2) associativity means that flow_dirn_ij = df/d(sig_ij) = s_ij*sqrt(3/2/(s_ij*s_ij)), and
 *     this means that flow_dirn_ij*flow_dirn_ij = 3/2, so when resid_ij=0, we get
 *     (plastic_strain_dot)_ij*(plastic_strain_dot)_ij = (3/2)*flow_incr^2, where
 *     plastic_strain_dot = plastic_strain - plastic_strain_old
 * (3) The definition of equivalent plastic strain is through
 *     eqvpstrain_dot = sqrt(2*plastic_strain_dot_ij*plastic_strain_dot_ij/3), so
 *     using (2), we obtain eqvpstrain_dot = flow_incr, and this yields
 *     internalPotential = -1 in the "rep" equation.
 */
void
FiniteStrainPlasticMaterial::returnMap(const RankTwoTensor & sig_old,
                                       const Real eqvpstrain_old,
                                       const RankTwoTensor & plastic_strain_old,
                                       const RankTwoTensor & delta_d,
                                       const RankFourTensor & E_ijkl,
                                       RankTwoTensor & sig,
                                       Real & eqvpstrain,
                                       RankTwoTensor & plastic_strain)
{
  // the yield function, must be non-positive
  // Newton-Raphson sets this to zero if trial stress enters inadmissible region
  Real f;

  // the consistency parameter, must be non-negative
  // change in plastic strain in this timestep = flow_incr*flow_potential
  Real flow_incr = 0.0;

  // direction of flow defined by the potential
  RankTwoTensor flow_dirn;

  // Newton-Raphson sets this zero
  // resid_ij = flow_incr*flow_dirn_ij - (plastic_strain - plastic_strain_old)
  RankTwoTensor resid;

  // Newton-Raphson sets this zero
  // rep = -flow_incr*internalPotential - (eqvpstrain - eqvpstrain_old)
  Real rep;

  // change in the stress (sig) in a Newton-Raphson iteration
  RankTwoTensor ddsig;

  // change in the consistency parameter in a Newton-Raphson iteration
  Real dflow_incr = 0.0;

  // change in equivalent plastic strain in one Newton-Raphson iteration
  Real deqvpstrain = 0.0;

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(trial_stress - sig), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp;

  // d(yieldFunction)/d(stress)
  RankTwoTensor df_dsig;

  // d(resid_ij)/d(sigma_kl)
  RankFourTensor dr_dsig;

  // dr_dsig_inv_ijkl*dr_dsig_klmn = 0.5*(de_ij de_jn + de_ij + de_jm), where de_ij = 1 if i=j, but
  // zero otherwise
  RankFourTensor dr_dsig_inv;

  // d(yieldFunction)/d(eqvpstrain)
  Real fq;

  // yield stress at the start of this "time step" (ie, evaluated with
  // eqvpstrain_old).  It is held fixed during the Newton-Raphson return,
  // even if eqvpstrain != eqvpstrain_old.
  Real yield_stress;

  // measures of whether the Newton-Raphson process has converged
  Real err1, err2, err3;

  // number of Newton-Raphson iterations performed
  unsigned int iter = 0;

  // maximum number of Newton-Raphson iterations allowed
  unsigned int maxiter = 100;

  // plastic loading occurs if yieldFunction > toly
  Real toly = 1.0e-8;

  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  sig = sig_old + E_ijkl * delta_d; // the trial stress
  eqvpstrain = eqvpstrain_old;
  plastic_strain = plastic_strain_old;

  yield_stress = getYieldStress(eqvpstrain); // yield stress at this equivalent plastic strain
  if (yieldFunction(sig, yield_stress) > toly)
  {
    // the sig just calculated is inadmissable.  We must return to the yield surface.
    // This is done iteratively, using a Newton-Raphson process.

    delta_dp.zero();

    sig = sig_old + E_ijkl * delta_d; // this is the elastic predictor

    flow_dirn = flowPotential(sig);

    resid = flow_dirn * flow_incr - delta_dp; // Residual 1 - refer Hughes Simo
    f = yieldFunction(sig, yield_stress);
    rep = -eqvpstrain + eqvpstrain_old - flow_incr * internalPotential(); // Residual 3 rep=0

    err1 = resid.L2norm();
    err2 = std::abs(f);
    err3 = std::abs(rep);

    while ((err1 > _rtol || err2 > _ftol || err3 > _eptol) &&
           iter < maxiter) // Stress update iteration (hardness fixed)
    {
      iter++;

      df_dsig = dyieldFunction_dstress(sig);
      getJac(sig, E_ijkl, flow_incr, dr_dsig);   // gets dr_dsig = d(resid_ij)/d(sig_kl)
      fq = dyieldFunction_dinternal(eqvpstrain); // d(f)/d(eqvpstrain)

      /**
       * The linear system is
       *   ( dr_dsig  flow_dirn  0  )( ddsig       )   ( - resid )
       *   ( df_dsig     0       fq )( dflow_incr  ) = ( - f     )
       *   (   0         1       -1 )( deqvpstrain )   ( - rep   )
       * The zeroes are: d(resid_ij)/d(eqvpstrain) = flow_dirn*d(df/d(sig_ij))/d(eqvpstrain) = 0
       * and df/d(flow_dirn) = 0  (this is always true, even for general hardening and
       * non-associative)
       * and d(rep)/d(sig_ij) = -flow_incr*d(internalPotential)/d(sig_ij) = 0
       */

      dr_dsig_inv = dr_dsig.invSymm();

      /**
       * Because of the zeroes and ones, the linear system is not impossible to
       * solve by hand.
       * NOTE: andy believes there was originally a sign-error in the next line.  The
       *       next line is unchanged, however andy's definition of fq is negative of
       *       the original definition of fq.  andy can't see any difference in any tests!
       */
      dflow_incr = (f - df_dsig.doubleContraction(dr_dsig_inv * resid) + fq * rep) /
                   (df_dsig.doubleContraction(dr_dsig_inv * flow_dirn) - fq);
      ddsig =
          dr_dsig_inv *
          (-resid -
           flow_dirn * dflow_incr); // from solving the top row of linear system, given dflow_incr
      deqvpstrain =
          rep + dflow_incr; // from solving the bottom row of linear system, given dflow_incr

      // update the variables
      flow_incr += dflow_incr;
      delta_dp -= E_ijkl.invSymm() * ddsig;
      sig += ddsig;
      eqvpstrain += deqvpstrain;

      // evaluate the RHS equations ready for next Newton-Raphson iteration
      flow_dirn = flowPotential(sig);
      resid = flow_dirn * flow_incr - delta_dp;
      f = yieldFunction(sig, yield_stress);
      rep = -eqvpstrain + eqvpstrain_old - flow_incr * internalPotential();

      err1 = resid.L2norm();
      err2 = std::abs(f);
      err3 = std::abs(rep);
    }

    if (iter >= maxiter)
      mooseError("Constitutive failure");

    plastic_strain += delta_dp;
  }
}

Real
FiniteStrainPlasticMaterial::yieldFunction(const RankTwoTensor & stress, const Real yield_stress)
{
  return getSigEqv(stress) - yield_stress;
}

RankTwoTensor
FiniteStrainPlasticMaterial::dyieldFunction_dstress(const RankTwoTensor & sig)
{
  RankTwoTensor deriv = sig.dsecondInvariant();
  deriv *= std::sqrt(3.0 / sig.secondInvariant()) / 2.0;
  return deriv;
}

Real
FiniteStrainPlasticMaterial::dyieldFunction_dinternal(const Real equivalent_plastic_strain)
{
  return -getdYieldStressdPlasticStrain(equivalent_plastic_strain);
}

RankTwoTensor
FiniteStrainPlasticMaterial::flowPotential(const RankTwoTensor & sig)
{
  return dyieldFunction_dstress(sig); // this plasticity model assumes associative flow
}

Real
FiniteStrainPlasticMaterial::internalPotential()
{
  return -1;
}

Real
FiniteStrainPlasticMaterial::getSigEqv(const RankTwoTensor & stress)
{
  return std::sqrt(3 * stress.secondInvariant());
}

// Jacobian for stress update algorithm
void
FiniteStrainPlasticMaterial::getJac(const RankTwoTensor & sig,
                                    const RankFourTensor & E_ijkl,
                                    Real flow_incr,
                                    RankFourTensor & dresid_dsig)
{
  RankTwoTensor sig_dev, df_dsig, flow_dirn;
  RankTwoTensor dfi_dft, dfi_dsig;
  RankFourTensor dft_dsig, dfd_dft, dfd_dsig;
  Real sig_eqv;
  Real f1, f2, f3;
  RankFourTensor temp;

  sig_dev = sig.deviatoric();
  sig_eqv = getSigEqv(sig);
  df_dsig = dyieldFunction_dstress(sig);
  flow_dirn = flowPotential(sig);

  f1 = 3.0 / (2.0 * sig_eqv);
  f2 = f1 / 3.0;
  f3 = 9.0 / (4.0 * Utility::pow<3>(sig_eqv));

  dft_dsig = f1 * _deltaMixed - f2 * _deltaOuter - f3 * sig_dev.outerProduct(sig_dev);

  dfd_dsig = dft_dsig;
  dresid_dsig = E_ijkl.invSymm() + dfd_dsig * flow_incr;
}

// Obtain yield stress for a given equivalent plastic strain (input)
Real
FiniteStrainPlasticMaterial::getYieldStress(const Real eqpe)
{
  unsigned nsize;

  nsize = _yield_stress_vector.size();

  if (_yield_stress_vector[0] > 0.0 || nsize % 2 > 0) // Error check for input inconsitency
    mooseError("Error in yield stress input: Should be a vector with eqv plastic strain and yield "
               "stress pair values.\n");

  unsigned int ind = 0;
  Real tol = 1e-8;

  while (ind < nsize)
  {
    if (std::abs(eqpe - _yield_stress_vector[ind]) < tol)
      return _yield_stress_vector[ind + 1];

    if (ind + 2 < nsize)
    {
      if (eqpe > _yield_stress_vector[ind] && eqpe < _yield_stress_vector[ind + 2])
        return _yield_stress_vector[ind + 1] +
               (eqpe - _yield_stress_vector[ind]) /
                   (_yield_stress_vector[ind + 2] - _yield_stress_vector[ind]) *
                   (_yield_stress_vector[ind + 3] - _yield_stress_vector[ind + 1]);
    }
    else
      return _yield_stress_vector[nsize - 1];

    ind += 2;
  }

  return 0.0;
}

Real
FiniteStrainPlasticMaterial::getdYieldStressdPlasticStrain(const Real eqpe)
{
  unsigned nsize;

  nsize = _yield_stress_vector.size();

  if (_yield_stress_vector[0] > 0.0 || nsize % 2 > 0) // Error check for input inconsitency
    mooseError("Error in yield stress input: Should be a vector with eqv plastic strain and yield "
               "stress pair values.\n");

  unsigned int ind = 0;

  while (ind < nsize)
  {
    if (ind + 2 < nsize)
    {
      if (eqpe >= _yield_stress_vector[ind] && eqpe < _yield_stress_vector[ind + 2])
        return (_yield_stress_vector[ind + 3] - _yield_stress_vector[ind + 1]) /
               (_yield_stress_vector[ind + 2] - _yield_stress_vector[ind]);
    }
    else
      return 0.0;

    ind += 2;
  }

  return 0.0;
}
