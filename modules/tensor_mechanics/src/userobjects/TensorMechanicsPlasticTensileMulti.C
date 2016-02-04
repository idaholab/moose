/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticTensileMulti.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileMulti>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Associative tensile plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addParam<Real>("shift", "Yield surface is shifted by this amount to avoid problems with defining derivatives when eigenvalues are equal.  If this is larger than f_tol, a warning will be issued.  Default = f_tol.");
  params.addParam<unsigned int>("max_iterations", 10, "Maximum number of Newton-Raphson iterations allowed in the custom return-map algorithm.  For highly nonlinear hardening this may need to be higher than 10.");
  params.addParam<bool>("use_custom_returnMap", true, "Whether to use the custom returnMap algorithm.  Set to true if you are using isotropic elasticity.");
  params.addParam<bool>("use_custom_cto", true, "Whether to use the custom consistent tangent operator computations.  Set to true if you are using isotropic elasticity.");
  return params;
}

TensorMechanicsPlasticTensileMulti::TensorMechanicsPlasticTensileMulti(const InputParameters & parameters) :
    TensorMechanicsPlasticModel(parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap")),
    _use_custom_cto(getParam<bool>("use_custom_cto")),
    _max_iters(getParam<unsigned int>("max_iterations")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : _f_tol)
{
  if (_shift < 0)
    mooseError("Value of 'shift' in TensorMechanicsPlasticTensileMulti must not be negative\n");
  if (_shift > _f_tol)
    _console << "WARNING: value of 'shift' in TensorMechanicsPlasticTensileMulti is probably set too high\n";
  MooseRandom::seed(0);
}

unsigned int
TensorMechanicsPlasticTensileMulti::numberSurfaces() const
{
  return 3;
}


void
TensorMechanicsPlasticTensileMulti::yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  Real str = tensile_strength(intnl);

  f.resize(3);
  f[0] = eigvals[0] + _shift - str;
  f[1] = eigvals[1] - str;
  f[2] = eigvals[2] - _shift - str;
}


void
TensorMechanicsPlasticTensileMulti::dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankTwoTensor> & df_dstress) const
{
  std::vector<Real> eigvals;
  stress.dsymmetricEigenvalues(eigvals, df_dstress);

  if (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
  {
    Real small_perturbation;
    RankTwoTensor shifted_stress = stress;
    while (eigvals[0] > eigvals[1] - 0.1*_shift || eigvals[1] > eigvals[2] - 0.1*_shift)
    {
      for (unsigned i = 0 ; i < 3 ; ++i)
        for (unsigned j = 0 ; j <= i ; ++j)
        {
          small_perturbation = 0.1*_shift*2*(MooseRandom::rand() - 0.5);
          shifted_stress(i, j) += small_perturbation;
          shifted_stress(j, i) += small_perturbation;
        }
      shifted_stress.dsymmetricEigenvalues(eigvals, df_dstress);
    }
  }
}


void
TensorMechanicsPlasticTensileMulti::dyieldFunction_dintnlV(const RankTwoTensor & /*stress*/, const Real & intnl, std::vector<Real> & df_dintnl) const
{
  df_dintnl.assign(3, -dtensile_strength(intnl));
}


void
TensorMechanicsPlasticTensileMulti::flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const
{
  // This plasticity is associative so
  dyieldFunction_dstressV(stress, intnl, r);
}


void
TensorMechanicsPlasticTensileMulti::dflowPotential_dstressV(const RankTwoTensor & stress, const Real & /*intnl*/, std::vector<RankFourTensor> & dr_dstress) const
{
  stress.d2symmetricEigenvalues(dr_dstress);
}


void
TensorMechanicsPlasticTensileMulti::dflowPotential_dintnlV(const RankTwoTensor & /*stress*/, const Real & /*intnl*/, std::vector<RankTwoTensor> & dr_dintnl) const
{
  dr_dintnl.assign(3, RankTwoTensor());
}

Real
TensorMechanicsPlasticTensileMulti::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticTensileMulti::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}


void
TensorMechanicsPlasticTensileMulti::activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const
{
  act.assign(3, false);

  if (f[0] <= _f_tol && f[1] <= _f_tol && f[2] <= _f_tol)
  {
    returned_stress = stress;
    return;
  }

  returned_stress = RankTwoTensor();

  std::vector<Real> eigvals;
  RankTwoTensor eigvecs;
  stress.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real str = tensile_strength(intnl);
  std::vector<Real> v(3);
  v[0] = eigvals[0] - str;
  v[1] = eigvals[1] - str;
  v[2] = eigvals[2] - str;

  // these are the normals to the 3 yield surfaces
  std::vector<std::vector<Real> > n(3);
  n[0].resize(3);
  n[0][0] = 1 ; n[0][1] = 0 ; n[0][2] = 0;
  n[1].resize(3);
  n[1][0] = 0 ; n[1][1] = 1 ; n[1][2] = 0;
  n[2].resize(3);
  n[2][0] = 0 ; n[2][1] = 0 ; n[2][2] = 1;

  // the flow directions are these n multiplied by Eijkl.
  // I re-use the name "n" for the flow directions
  // In the following I assume that the Eijkl is
  // for an isotropic situation.  This is the most
  // common when using TensileMulti, and remember
  // that the returned_stress need not be perfect
  // anyway.
  // I divide by E(0,0,0,0) so the n remain of order 1
  Real ratio = Eijkl(1,1,0,0)/Eijkl(0,0,0,0);
  n[0][1] = n[0][2] = ratio;
  n[1][0] = n[1][2] = ratio;
  n[2][0] = n[2][1] = ratio;


  // 111 (tip)
  // For tip-return to satisfy Kuhn-Tucker, we need
  // v = alpha*n[0] + beta*n[1] * gamma*n[2]
  // with alpha, beta, and gamma all being non-negative (they are
  // the plasticity multipliers)
  Real denom = triple(n[0], n[1], n[2]);
  if (triple(v, n[0], n[1])/denom >= 0 && triple(v, n[1], n[2])/denom >= 0 && triple(v, n[2], n[0])/denom >= 0)
  {
    act[0] = act[1] = act[2] = true;
    returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = str;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    return;
  }

  // 011 (edge)
  std::vector<Real> n1xn2(3);
  n1xn2[0] = n[1][1]*n[2][2] - n[1][2]*n[2][1];
  n1xn2[1] = n[1][2]*n[2][0] - n[1][0]*n[2][2];
  n1xn2[2] = n[1][0]*n[2][1] - n[1][1]*n[2][0];
  // work out the point to which we would return, "a".  It is defined by
  // f1 = 0 = f2, and that (p - a).(n1 x n2) = 0, where "p" is the
  // starting position (p = eigvals).
  // In the following a = (a0, str, str)
  Real pdotn1xn2 = dot(eigvals, n1xn2);
  Real a0 = (-str*n1xn2[1] - str*n1xn2[2] + pdotn1xn2)/n1xn2[0];
  // we need p - a = alpha*n1 + beta*n2, where alpha and beta are non-negative
  // for Kuhn-Tucker to be satisfied
  std::vector<Real> pminusa(3);
  pminusa[0] = eigvals[0] - a0;
  pminusa[1] = v[1];
  pminusa[2] = v[2];
  if ((pminusa[2] - pminusa[0])/(1.0 - ratio) >= 0 && (pminusa[1] - pminusa[0])/(1.0 - ratio) >= 0)
  {
    returned_stress(0, 0) = a0;
    returned_stress(1, 1) = str;
    returned_stress(2, 2) = str;
    returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
    act[1] = act[2] = true;
    return;
  }

  // 001 (plane)
  // the returned point, "a", is defined by f2=0 and
  // a = p - alpha*n2
  Real alpha = (eigvals[2] - str)/n[2][2];
  act[2] = true;
  returned_stress(0, 0) = eigvals[0] - alpha*n[2][0];
  returned_stress(1, 1) = eigvals[1] - alpha*n[2][1];
  returned_stress(2, 2) = str;
  returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
  return;
}

Real
TensorMechanicsPlasticTensileMulti::dot(const std::vector<Real> & a, const std::vector<Real> & b) const
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

Real
TensorMechanicsPlasticTensileMulti::triple(const std::vector<Real> & a, const std::vector<Real> & b, const std::vector<Real> & c) const
{
  return a[0]*(b[1]*c[2] - b[2]*c[1]) - a[1]*(b[0]*c[2] - b[2]*c[0]) + a[2]*(b[0]*c[1] - b[1]*c[0]);
}

std::string
TensorMechanicsPlasticTensileMulti::modelName() const
{
  return "TensileMulti";
}


bool
TensorMechanicsPlasticTensileMulti::returnMap(const RankTwoTensor & trial_stress, const Real & intnl_old, const RankFourTensor & E_ijkl,
                                              Real ep_plastic_tolerance, RankTwoTensor & returned_stress, Real & returned_intnl,
                                std::vector<Real> & dpm, RankTwoTensor & delta_dp, std::vector<Real> & yf,
                                bool & trial_stress_inadmissible) const
{
  if (!_use_custom_returnMap)
    return TensorMechanicsPlasticModel::returnMap(trial_stress, intnl_old, E_ijkl,
                                                  ep_plastic_tolerance, returned_stress, returned_intnl,
                                                  dpm, delta_dp, yf, trial_stress_inadmissible);

  mooseAssert(dpm.size() == 3, "TensorMechanicsPlasticTensileMulti size of dpm should be 3 but it is " << dpm.size());

  std::vector<Real> eigvals;
  RankTwoTensor eigvecs;
  trial_stress.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real str = tensile_strength(intnl_old);

  yf.resize(3);
  yf[0] = eigvals[0] - str;
  yf[1] = eigvals[1] - str;
  yf[2] = eigvals[2] - str;

  if (yf[0] <= _f_tol && yf[1] <= _f_tol && yf[2] <= _f_tol)
  {
    // purely elastic (trial_stress, intnl_old)
    trial_stress_inadmissible = false;
    return true;
  }

  trial_stress_inadmissible = true;
  delta_dp.zero();
  returned_stress.zero();

  // In the following i often assume that E_ijkl is
  // for an isotropic situation.  This reduces FLOPS
  // substantially which is important since the returnMap
  // is potentially the most compute-intensive function
  // of a simulation.
  // In many comments i write the general expression, and
  // i hope that might guide future coders if they are
  // generalising to a non-istropic E_ijkl

  // n[alpha] = E_ijkl*r[alpha]_kl expressed in principal stress space
  // (alpha = 0, 1, 2, corresponding to the three surfaces)
  // Note that in principal stress space, the flow
  // directions are, expressed in 'vector' form,
  // r[0] = (1,0,0), r[1] = (0,1,0), r[2] = (0,0,1).
  // Similar for _n:
  // so _n[0] = E_ij00*r[0], _n[1] = E_ij11*r[1], _n[2] = E_ij22*r[2]
  // In the following I assume that the E_ijkl is
  // for an isotropic situation.
  // In the anisotropic situation, we couldn't express
  // the flow directions as vectors in the same principal
  // stress space as the stress: they'd be full rank-2 tensors
  std::vector<std::vector<Real> > n(3);
  for (unsigned i = 0 ; i < 3 ; ++i)
    n[i].resize(3);
  n[0][0] = E_ijkl(0,0,0,0);
  n[0][1] = E_ijkl(1,1,0,0);
  n[0][2] = E_ijkl(2,2,0,0);
  n[1][0] = E_ijkl(0,0,1,1);
  n[1][1] = E_ijkl(1,1,1,1);
  n[1][2] = E_ijkl(2,2,1,1);
  n[2][0] = E_ijkl(0,0,2,2);
  n[2][1] = E_ijkl(1,1,2,2);
  n[2][2] = E_ijkl(2,2,2,2);


  // With non-zero Poisson's ratio and hardening
  // it is not computationally cheap to know whether
  // the trial stress will return to the tip, edge,
  // or plane.  The following is correct for zero
  // Poisson's ratio and no hardening, and at least
  // gives a not-completely-stupid guess in the
  // more general case.
  // trial_order[0] = type of return to try first
  // trial_order[1] = type of return to try second
  // trial_order[2] = type of return to try third
  std::vector<int> trial_order(3);
  if (yf[0] > 0) // all the yield functions are positive, since eigvals are ordered eigvals[0] <= eigvals[1] <= eigvals[2]
  {
    trial_order[0] = tip;
    trial_order[1] = edge;
    trial_order[2] = plane;
  }
  else if (yf[1] > 0)  // two yield functions are positive
  {
    trial_order[0] = edge;
    trial_order[1] = tip;
    trial_order[2] = plane;
  }
  else
  {
    trial_order[0] = plane;
    trial_order[1] = edge;
    trial_order[2] = tip;
  }

  unsigned trial;
  bool nr_converged;
  for (trial = 0 ; trial < 3 ; ++trial)
  {
    switch(trial_order[trial])
    {
    case(tip):
      nr_converged = returnTip(eigvals, n, dpm, returned_stress, intnl_old);
      break;
    case(edge):
      nr_converged = returnEdge(eigvals, n, dpm, returned_stress, intnl_old);
      break;
    case(plane):
      nr_converged = returnPlane(eigvals, n, dpm, returned_stress, intnl_old);
      break;
    }
    str = tensile_strength(intnl_old + dpm[0] + dpm[1] + dpm[2]);
    if (nr_converged && KuhnTuckerOK(returned_stress, dpm, str))
      break;
  }

  if (trial == 3)
  {
    Moose::out << "Trial stress = \n";
    trial_stress.print();
    mooseError("TensorMechanicsPlasticTensileMulti: FAILURE!\n");
    // failure - must place yield function values at trial stress into yf
    str = tensile_strength(intnl_old);
    yf[0] = eigvals[0] - str;
    yf[1] = eigvals[1] - str;
    yf[2] = eigvals[2] - str;
    return false;
  }

  // success

  returned_intnl = intnl_old;
  for (unsigned i = 0 ; i < 3 ; ++i)
  {
    yf[i] = returned_stress(i, i) - str;
    delta_dp(i, i) = dpm[i];
    returned_intnl += dpm[i];
  }
  returned_stress = eigvecs*returned_stress*(eigvecs.transpose());
  delta_dp = eigvecs*delta_dp*(eigvecs.transpose());

  RankFourTensor dummmm = consistentTangentOperator(trial_stress, returned_stress, returned_intnl, E_ijkl, dpm);
  return true;
}

bool
TensorMechanicsPlasticTensileMulti::returnTip(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n, std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const
{
  // The returned point is defined by f0=f1=f2=0.
  // that is, returned_stress = diag(str, str, str), where
  // str = tensile_strength(intnl),
  // where intnl = intnl_old + dpm[0] + dpm[1] + dpm[2]
  // The 3 plastic multipliers, dpm, are defiend by the normality condition
  //   eigvals - str = dpm[0]*n[0] + dpm[1]*n[1] + dpm[2]*n[2]
  // (Kuhn-Tucker demands that all dpm are non-negative, but we leave
  // that checking for later.)
  // This is a vector equation with solution (A):
  //   dpm[0] = triple(eigvals - str, n[1], n[2])/trip;
  //   dpm[1] = triple(eigvals - str, n[2], n[0])/trip;
  //   dpm[2] = triple(eigvals - str, n[0], n[1])/trip;
  // where trip = triple(n[0], n[1], n[2]).
  // By adding the three components of that solution together
  // we can get an equation for x = dpm[0] + dpm[1] + dpm[2],
  // and then our Newton-Raphson only involves one variable (x).
  // In the following, i specialise to the isotropic situation.

  Real x = 0;
  Real denom = (n[0][0] - n[0][1])*(n[0][0] + 2*n[0][1]);
  Real str = tensile_strength(intnl_old + x);

  if (_strength.modelName().compare("Constant") != 0)
  {
    // Finding x is expensive.  Therefore
    // although x!=0 for Constant Hardening, solution (A)
    // demonstrates that we don't
    // actually need to know x to find the dpm for
    // Constant Hardening.
    //
    // However, for nontrivial Hardening, the following
    // is necessary
    Real eig = eigvals[0] + eigvals[1] + eigvals[2];
    Real bul = (n[0][0] + 2*n[0][1]);

    // and finally, the equation we want to solve is:
    // bul*x - eig + 3*str = 0
    // where str=tensile_strength(intnl_old + x)
    // and x = dpm[0] + dpm[1] + dpm[2]
    // (Note this has units of stress, so using _f_tol as a convergence check is reasonable.)
    // Use Netwon-Raphson with initial guess x = 0
    Real residual = bul*x - eig + 3*str;
    Real jacobian;
    unsigned int iter = 0;
    do {
      jacobian = bul + 3*dtensile_strength(intnl_old + x);
      x += -residual/jacobian;
      if (iter > _max_iters) // not converging
        return false;
      str = tensile_strength(intnl_old + x);
      residual = bul*x - eig + 3*str;
      iter ++;
    } while (residual*residual > _f_tol*_f_tol);
  }

  // The following is the solution (A) written above
  // (dpm[0] = triple(eigvals - str, n[1], n[2])/trip, etc)
  // in the isotropic situation
  dpm[0] = (n[0][0]*(eigvals[0] - str) + n[0][1]*(eigvals[0] - eigvals[1] - eigvals[2] + str))/denom;
  dpm[1] = (n[0][0]*(eigvals[1] - str) + n[0][1]*(eigvals[1] - eigvals[2] - eigvals[0] + str))/denom;
  dpm[2] = (n[0][0]*(eigvals[2] - str) + n[0][1]*(eigvals[2] - eigvals[0] - eigvals[1] + str))/denom;
  returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = str;
  return true;
}

bool
TensorMechanicsPlasticTensileMulti::returnEdge(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n, std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const
{
  // work out the point to which we would return, "a".  It is defined by
  // f1 = 0 = f2, and the normality condition:
  //   (eigvals - a).(n1 x n2) = 0,
  // where eigvals is the starting position
  // (it is a vector in principal stress space).
  // To get f1=0=f2, we need a = (a0, str, str), and a0 is found
  // by expanding the normality condition to yield:
  //   a0 = (-str*n1xn2[1] - str*n1xn2[2] + edotn1xn2)/n1xn2[0];
  // where edotn1xn2 = eigvals.(n1 x n2)
  //
  // We need to find the plastic multipliers, dpm, defined by
  //   eigvals - a = dpm[1]*n1 + dpm[2]*n2
  // For the isotropic case, and defining eminusa = eigvals - a,
  // the solution is easy:
  //   dpm[0] = 0;
  //   dpm[1] = (eminusa[1] - eminusa[0])/(n[1][1] - n[1][0]);
  //   dpm[2] = (eminusa[2] - eminusa[0])/(n[2][2] - n[2][0]);
  //
  // Now specialise to the isotropic case.  Define
  //   x = dpm[1] + dpm[2] = (eigvals[1] + eigvals[2] - 2*str)/(n[0][0] + n[0][1])
  // Notice that the RHS is a function of x, so we solve using
  // Newton-Raphson starting with x=0
  Real x = 0;
  Real denom = n[0][0] + n[0][1];
  Real str = tensile_strength(intnl_old + x);

  if (_strength.modelName().compare("Constant") != 0)
  {
    // Finding x is expensive.  Therefore
    // although x!=0 for Constant Hardening, solution
    // for dpm above demonstrates that we don't
    // actually need to know x to find the dpm for
    // Constant Hardening.
    //
    // However, for nontrivial Hardening, the following
    // is necessary
    Real eig = eigvals[1] + eigvals[2];
    Real residual = denom*x - eig + 2*str;
    Real jacobian;
    unsigned int iter = 0;
    do {
      jacobian = denom + 2*dtensile_strength(intnl_old + x);
      x += -residual/jacobian;
      if (iter > _max_iters) // not converging
        return false;
      str = tensile_strength(intnl_old + x);
      residual = denom*x - eig + 2*str;
      iter ++;
    } while (residual*residual > _f_tol*_f_tol);
  }

  dpm[0] = 0;
  dpm[1] = ((eigvals[1]*n[0][0] - eigvals[2]*n[0][1])/(n[0][0] - n[0][1]) - str)/denom;
  dpm[2] = ((eigvals[2]*n[0][0] - eigvals[1]*n[0][1])/(n[0][0] - n[0][1]) - str)/denom;

  returned_stress(0, 0) = eigvals[0] - n[0][1]*(dpm[1] + dpm[2]);
  returned_stress(1, 1) = returned_stress(2, 2) = str;
  return true;
}

bool
TensorMechanicsPlasticTensileMulti::returnPlane(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n, std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const
{
  // the returned point, "a", is defined by f2=0 and
  // a = p - dpm[2]*n2.
  // This is a vector equation in
  // principal stress space, and dpm[2] is the third
  // plasticity multiplier (dpm[0]=0=dpm[1] for return
  // to the plane) and "p" is the starting
  // position (p=eigvals).
  // (Kuhn-Tucker demands that dpm[2]>=0, but we leave checking
  // that condition for later.)
  // Since f2=0, we must have a[2]=tensile_strength,
  // so we can just look at the [2] component of the
  // equation, which yields
  // n[2][2]*dpm[2] - eigvals[2] + str = 0
  // For hardening, str=tensile_strength(intnl_old+dpm[2]),
  // and we want to solve for dpm[2].
  // Use Newton-Raphson with initial guess dpm[2] = 0
  dpm[2] = 0;
  Real residual = n[2][2]*dpm[2] - eigvals[2] + tensile_strength(intnl_old + dpm[2]);
  Real jacobian;
  unsigned int iter = 0;
  do {
    jacobian = n[2][2] + dtensile_strength(intnl_old + dpm[2]);
    dpm[2] += -residual/jacobian;
    if (iter > _max_iters) // not converging
      return false;
    residual = n[2][2]*dpm[2] - eigvals[2] + tensile_strength(intnl_old + dpm[2]);
    iter ++;
  } while (residual*residual > _f_tol*_f_tol);

  dpm[0] = 0;
  dpm[1] = 0;
  returned_stress(0, 0) = eigvals[0] - dpm[2]*n[2][0];
  returned_stress(1, 1) = eigvals[1] - dpm[2]*n[2][1];
  returned_stress(2, 2) = eigvals[2] - dpm[2]*n[2][2];
  return true;
}

bool
TensorMechanicsPlasticTensileMulti::KuhnTuckerOK(const RankTwoTensor & returned_diagonal_stress, const std::vector<Real> & dpm, const Real & str) const
{
  for (unsigned i = 0 ; i < 3 ; ++i)
    if (!TensorMechanicsPlasticModel::KuhnTuckerSingleSurface(returned_diagonal_stress(i, i) - str, dpm[i]))
      return false;
  return true;
}


RankFourTensor
TensorMechanicsPlasticTensileMulti::consistentTangentOperator(const RankTwoTensor & trial_stress, const RankTwoTensor & stress, const Real & intnl,
                                                       const RankFourTensor & E_ijkl, const std::vector<Real> & cumulative_pm) const
{
  if (!_use_custom_cto)
    return TensorMechanicsPlasticModel::consistentTangentOperator(trial_stress, stress, intnl, E_ijkl, cumulative_pm);

  mooseAssert(cumulative_pm.size() == 3, "TensorMechanicsPlasticTensileMulti size of cumulative_pm should be 3 but it is " << cumulative_pm.size());

  if (cumulative_pm[2] <= 0) // All cumulative_pm are non-positive, so this is admissible
    return E_ijkl;


  // Need the eigenvalues at the returned configuration
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);

  // need to rotate to and from principal stress space
  // using the eigenvectors of the trial configuration
  // (not the returned configuration).
  std::vector<Real> trial_eigvals;
  RankTwoTensor trial_eigvecs;
  trial_stress.symmetricEigenvaluesEigenvectors(trial_eigvals, trial_eigvecs);

  // The returnMap will have returned to the Tip, Edge or
  // Plane.  The consistentTangentOperator describes the
  // change in stress for an arbitrary change in applied
  // strain.  I assume that the change in strain will not
  // change the type of return (Tip remains Tip, Edge remains
  // Edge, Plane remains Plane).
  // I assume isotropic elasticity.
  //
  // The consistent tangent operator is a little different
  // than cases where no rotation to principal stress space
  // is made during the returnMap.  Let S_ij be the stress
  // in original coordinates, and s_ij be the stress in the
  // principal stress coordinates, so that
  // s_ij = diag(eigvals[0], eigvals[1], eigvals[2])
  // We want dS_ij under an arbitrary change in strain (ep->ep+dep)
  // dS = S(ep+dep) - S(ep)
  //    = R(ep+dep) s(ep+dep) R(ep+dep)^T - R(ep) s(ep) R(ep)^T
  // Here R = the rotation to principal-stress space, ie
  // R_ij = eigvecs[i][j] = i^th component of j^th eigenvector
  // Expanding to first order in dep,
  // dS = R(ep) (s(ep+dep) - s(ep)) R(ep)^T
  //      + dR/dep s(ep) R^T + R(ep) s(ep) dR^T/dep
  // The first line is all that is usually calculated in the
  // consistent tangent operator calculation, and is called
  // cto below.
  // The second line involves changes in the eigenvectors, and
  // is called sec below.

  RankFourTensor cto;
  Real hard = dtensile_strength(intnl);
  Real la = E_ijkl(0,0,1,1);
  Real mu = 0.5*(E_ijkl(0,0,0,0) - la);

  if (cumulative_pm[1] <= 0)
  {
    // only cumulative_pm[2] is positive, so this is return to the Plane
    Real denom = hard + la + 2*mu;
    Real al = la*la/denom;
    Real be = la*(la + 2*mu)/denom;
    Real ga = hard*(la + 2*mu)/denom;
    std::vector<Real> comps(9);
    comps[0] = comps[4] = la + 2*mu - al;
    comps[1] = comps[3] = la - al;
    comps[2] = comps[5] = comps[6] = comps[7] = la - be;
    comps[8] = ga;
    cto.fillFromInputVector(comps, RankFourTensor::principal);
  }
  else if (cumulative_pm[0] <= 0)
  {
    // both cumulative_pm[2] and cumulative_pm[1] are positive, so Edge
    Real denom = 2*hard + 2*la + 2*mu;
    Real al = hard*2*la/denom;
    Real be = hard*(2*la + 2*mu)/denom;
    std::vector<Real> comps(9);
    comps[0] = la + 2*mu - 2*la*la/denom;
    comps[1] = comps[2] = al;
    comps[3] = comps[6] = al;
    comps[4] = comps[5] = comps[7] = comps[8] = be;
    cto.fillFromInputVector(comps, RankFourTensor::principal);
  }
  else
  {
    // all cumulative_pm are positive, so Tip
    Real denom = 3*hard + 3*la + 2*mu;
    std::vector<Real> comps(2);
    comps[0] = hard*(3*la + 2*mu)/denom;
    comps[1] = 0;
    cto.fillFromInputVector(comps, RankFourTensor::symmetric_isotropic);
  }

  cto.rotate(trial_eigvecs);


  // drdsig = change in eigenvectors under a small stress change
  // drdsig(i,j,m,n) = dR(i,j)/dS_mn
  // The formula below is fairly easily derived:
  // S R = R s, so taking the variation
  // dS R + S dR = dR s + R ds, and multiplying by R^T
  // R^T dS R + R^T S dR = R^T dR s + ds .... (eqn 1)
  // I demand that RR^T = 1 = R^T R, and also that
  // (R+dR)(R+dR)^T = 1 = (R+dT)^T (R+dR), which means
  // that dR = R*c, for some antisymmetric c, so Eqn1 reads
  // R^T dS R + s c = c s + ds
  // Grabbing the components of this gives ds/dS (already
  // in RankTwoTensor), and c, which is:
  /*  RankFourTensor drdsig;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned k = 0 ; k < 3 ; ++k)
      for (unsigned m = 0 ; m < 3 ; ++m)
        for (unsigned n = 0 ; n < 3 ; ++n)
          for (unsigned b = 0 ; b < 3 ; ++b)
            {
              if (b == k)
                continue;
              drdsig(i, k, m, n) += trial_eigvecs(m, b)*trial_eigvecs(n, k)*trial_eigvecs(i, b)/(trial_eigvals[k] - trial_eigvals[b]);
            }
  */

  RankFourTensor drdsig;
  for (unsigned k = 0 ; k < 3 ; ++k)
    for (unsigned b = 0 ; b < 3 ; ++b)
    {
      if (b == k)
        continue;
      for (unsigned m = 0 ; m < 3 ; ++m)
        for (unsigned n = 0 ; n < 3 ; ++n)
          for (unsigned i = 0 ; i < 3 ; ++i)
            drdsig(i, k, m, n) += trial_eigvecs(m, b)*trial_eigvecs(n, k)*trial_eigvecs(i, b)/(trial_eigvals[k] - trial_eigvals[b]);
    }



  /*
  RankTwoTensor diagla;
  diagla(0, 0) = eigvals[0];
  diagla(1, 1) = eigvals[1];
  diagla(2, 2) = eigvals[2];

  RankFourTensor ans;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned a = 0 ; a < 3 ; ++a)
        for (unsigned b = 0 ; b < 3 ; ++b)
          for (unsigned k = 0 ; k < 3 ; ++k)
            for (unsigned l = 0 ; l < 3 ; ++l)
              for (unsigned m = 0 ; m < 3 ; ++m)
                for (unsigned n = 0 ; n < 3 ; ++n)
                  ans(i, j, a, b) += (drdsig(i, k, m, n)*trial_eigvecs(j, l)*diagla(k, l) + trial_eigvecs(i, k)*drdsig(j, l, m, n)*diagla(k, l))*E_ijkl(m, n, a, b);
  */

  RankFourTensor ans;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned a = 0 ; a < 3 ; ++a)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned m = 0 ; m < 3 ; ++m)
            ans(i, j, a, a) += (drdsig(i, k, m, m)*trial_eigvecs(j, k) + trial_eigvecs(i, k)*drdsig(j, k, m, m))*eigvals[k]*la;  //E_ijkl(m, n, a, b) = la*(m==n)*(a==b);

  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned a = 0 ; a < 3 ; ++a)
        for (unsigned b = 0 ; b < 3 ; ++b)
          for (unsigned k = 0 ; k < 3 ; ++k)
          {
            ans(i, j, a, b) += (drdsig(i, k, a, b)*trial_eigvecs(j, k) + trial_eigvecs(i, k)*drdsig(j, k, a, b))*eigvals[k]*mu;  //E_ijkl(m, n, a, b) = mu*(m==a)*(n==b)
            ans(i, j, a, b) += (drdsig(i, k, b, a)*trial_eigvecs(j, k) + trial_eigvecs(i, k)*drdsig(j, k, b, a))*eigvals[k]*mu;  //E_ijkl(m, n, a, b) = mu*(m==b)*(n==a)
          }


  return cto + ans;

}
