/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticDruckerPragerHyperbolic.h"

template<>
InputParameters validParams<TensorMechanicsPlasticDruckerPragerHyperbolic>()
{
  InputParameters params = validParams<TensorMechanicsPlasticDruckerPrager>();
  params.addParam<bool>("use_custom_returnMap", true, "Whether to use the custom returnMap algorithm.  Set to true if you are using isotropic elasticity.");
  params.addRangeCheckedParam<Real>("smoother", 0.0, "smoother>=0", "The cone vertex at J2=0 is smoothed.  The maximum mean stress possible, which is Cohesion*Cot(friction_angle) for smoother=0, becomes (Cohesion - smoother/3)*Cot(friction_angle).  This is a non-hardening parameter");
  params.addRangeCheckedParam<unsigned>("max_iterations", 40, "max_iterations>0", "Maximum iterations to use in the custom return map function");
  params.addClassDescription("Non-associative Drucker Prager plasticity with hyperbolic smoothing of the cone tip.");
  return params;
}

TensorMechanicsPlasticDruckerPragerHyperbolic::TensorMechanicsPlasticDruckerPragerHyperbolic(const InputParameters & parameters) :
    TensorMechanicsPlasticDruckerPrager(parameters),
    _smoother2(std::pow(getParam<Real>("smoother"), 2)),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap")),
    _max_iters(getParam<unsigned>("max_iterations"))
{
}

Real
TensorMechanicsPlasticDruckerPragerHyperbolic::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  Real aaa;
  Real bbb;
  AandB(intnl, aaa, bbb);
  return std::sqrt(stress.secondInvariant() + _smoother2) + stress.trace()*bbb - aaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPragerHyperbolic::df_dsig(const RankTwoTensor & stress, Real bbb) const
{
  return 0.5*stress.dsecondInvariant()/std::sqrt(stress.secondInvariant() + _smoother2) + stress.dtrace()*bbb;
}


RankFourTensor
TensorMechanicsPlasticDruckerPragerHyperbolic::dflowPotential_dstress(const RankTwoTensor & stress, Real /*intnl*/) const
{
  RankFourTensor dr_dstress;
  dr_dstress = 0.5*stress.d2secondInvariant()/std::sqrt(stress.secondInvariant() + _smoother2);
  dr_dstress += -0.5*0.5*stress.dsecondInvariant().outerProduct(stress.dsecondInvariant())/std::pow(stress.secondInvariant() + _smoother2, 1.5);
  return dr_dstress;
}

std::string
TensorMechanicsPlasticDruckerPragerHyperbolic::modelName() const
{
  return "DruckerPragerHyperbolic";
}


bool
TensorMechanicsPlasticDruckerPragerHyperbolic::returnMap(const RankTwoTensor & trial_stress,
                                                         Real intnl_old, const RankFourTensor & E_ijkl,
                                                         Real ep_plastic_tolerance, RankTwoTensor & returned_stress,
                                                         Real & returned_intnl, std::vector<Real> & dpm,
                                                         RankTwoTensor & delta_dp, std::vector<Real> & yf,
                                                         bool & trial_stress_inadmissible) const
{
  if (!(_use_custom_returnMap))
    return TensorMechanicsPlasticModel::returnMap(trial_stress, intnl_old, E_ijkl, ep_plastic_tolerance, returned_stress, returned_intnl, dpm, delta_dp, yf, trial_stress_inadmissible);

  yf.resize(1);

  Real yf_orig = yieldFunction(trial_stress, intnl_old);

  yf[0] = yf_orig;

  if (yf_orig < _f_tol)
  {
    // the trial_stress is admissible
    trial_stress_inadmissible = false;
    return true;
  }

  trial_stress_inadmissible = true;
  const Real mu = E_ijkl(0,1,0,1);
  const Real lambda = E_ijkl(0,0,0,0) - 2*mu;
  const Real bulky = 3*lambda + 2*mu;
  const Real Tr_trial = trial_stress.trace();
  const Real J2trial = trial_stress.secondInvariant();

  /* the correct answer is obtained:
  RankTwoTensor should_be_n = mu*trial_stress.deviatoric()/sqrt(trial_stress.secondInvariant());
  Real b_f;
  Bonly(intnl_old, false, b_f);
  for (unsigned i = 0 ; i < 3 ; ++i)
    should_be_n(i, i) += bulky*b_f;
  Moose::out << "should_be_n:\n";
  should_be_n.print();
  RankTwoTensor actual_n(E_ijkl*flowPotential(trial_stress, intnl_old));
  Moose::out << "actual_n:\n";
  actual_n.print();
  */

  // Perform a Newton-Raphson to find dpm when
  // residual = (1 + dpm*mu/ll)sqrt(ll^2 - s^2) - sqrt(J2trial) = 0, with s=smoother
  // with ll = sqrt(J2 + s^2) = aaa - bbb*Tr(stress) = aaa - bbb(Tr_trial - p*bulky*bbb_flow)
  Real aaa;
  Real daaa;
  Real bbb;
  Real dbbb;
  Real bbb_flow;
  Real dbbb_flow;
  Real ll;
  Real dll;
  Real residual;
  Real jac;
  dpm[0] = 0;
  unsigned int iter = 0;
  do {
    AandB(intnl_old + dpm[0], aaa, bbb);
    dAandB(intnl_old + dpm[0], daaa, dbbb);
    Bonly(intnl_old + dpm[0], false, bbb_flow);
    dBonly(intnl_old + dpm[0], false, dbbb_flow);
    ll = aaa - bbb*(Tr_trial - dpm[0]*bulky*3*bbb_flow);
    dll = daaa - dbbb*(Tr_trial - dpm[0]*bulky*3*bbb_flow) + bbb*bulky*3*(bbb_flow + dpm[0]*dbbb_flow);
    residual = bbb*(Tr_trial - dpm[0]*bulky*3*bbb_flow) - aaa + std::sqrt(J2trial/std::pow(1 + dpm[0]*mu/ll,2) + _smoother2);
    jac = dbbb*(Tr_trial - dpm[0]*bulky*3*bbb_flow) - bbb*bulky*3*(bbb_flow + dpm[0]*dbbb_flow) - daaa + 0.5*J2trial*(-2)*(mu/ll - dpm[0]*mu*dll/ll/ll)/std::pow(1 + dpm[0]*mu/ll, 3)/std::sqrt(J2trial/std::pow(1 + dpm[0]*mu/ll,2) + _smoother2);
    dpm[0] += -residual/jac;
    if (iter > _max_iters) // not converging
      return false;
    iter++;
  } while (residual*residual > _f_tol*_f_tol);

  // TODO : check the following stuff!!
  // set the returned values
  yf[0] = 0;
  returned_intnl = intnl_old + dpm[0];

  AandB(returned_intnl, aaa, bbb);
  Bonly(returned_intnl, false, bbb_flow);
  ll = aaa - bbb*(Tr_trial - dpm[0]*bulky*3*bbb_flow);
  returned_stress = trial_stress.deviatoric()/(1 + dpm[0]*mu/ll); // this is the deviatoric part only


  /*
  // form the flow vector at the returned stress:
  RankTwoTensor nij = mu*returned_stress/ll;
  for (unsigned i = 0 ; i < 3 ; ++i)
    nij(i, i) += bulky*bbb_flow;
  */

  // form the returned stress
  Real returned_trace_over_3 = (aaa - ll)/bbb/3.0;
  for (unsigned i = 0 ; i < 3 ; ++i)
    returned_stress(i, i) += returned_trace_over_3;

  RankTwoTensor rij = flowPotential(returned_stress, returned_intnl); // todo: this needs to be optimised
  delta_dp = rij*dpm[0];

  /* TODO: remove the following checks
  // nij = E_ijkl*rij
  Moose::out << "after n=\n";
  nij.print();
  Moose::out << "E_ijkl*rij=\n";
  (E_ijkl*rij).print();
  */

  RankTwoTensor scheck(returned_stress - trial_stress + dpm[0]*(E_ijkl*rij));
  if (scheck.L2norm() > _f_tol)
    Moose::out << "Oh dear, not properly normal.  |scheck| = " << scheck.L2norm() << "\n";

  // TODO: get rid of the following lines:
  yf[0] = yieldFunction(returned_stress, returned_intnl);
  if (std::abs(yf[0]) > _f_tol)
    Moose::out << "Oh dear, yf = " << yf[0] << "\n";
  if (dpm[0] < 0)
    Moose::out << "Of dear, dpm = " << dpm[0] << "\n";
  //RankTwoTensor fv(E_ijkl*flowPotential(returned_stress, returned_intnl) - nij);
  //Moose::out << " should be zero: " << fv.L2norm() << "\n";

  return true;
}

bool
TensorMechanicsPlasticDruckerPragerHyperbolic::useCustomReturnMap() const
{
  return _use_custom_returnMap;
}
