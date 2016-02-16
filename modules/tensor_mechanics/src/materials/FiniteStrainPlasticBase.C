/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FiniteStrainPlasticBase.h"

// Following is used to access PETSc's LAPACK routines
#include <petscblaslapack.h>

template<>
InputParameters validParams<FiniteStrainPlasticBase>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRangeCheckedParam<unsigned int>("max_NR_iterations", 20, "max_NR_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addRequiredParam<std::vector<Real> >("yield_function_tolerance", "If the yield function is less than this amount, the (stress, internal parameters) are deemed admissible.  A vector of tolerances must be entered for the multi-surface case");
  params.addParam<std::vector<Real> >("internal_constraint_tolerance", "The Newton-Raphson process is only deemed converged if the internal constraint is less than this.  A vector of tolerances must be entered for the case with more than one internal parameter");
  params.addRequiredRangeCheckedParam<Real>("ep_plastic_tolerance", "ep_plastic_tolerance>0", "The Newton-Raphson process is only deemed converged if the plastic strain increment constraints have L2 norm less than this.");
  params.addRangeCheckedParam<unsigned int>("max_subdivisions", 4096, "max_subdivisions>0", "If ordinary Newton-Raphson + line-search fails, then the applied strain increment is subdivided, and the return-map is tried again.  This parameter is the maximum number of subdivisions allowed.  The number of subdivisions tried increases exponentially, first 1, then 2, then 4, then 8, etc");
  params.addParam<int>("debug_fspb", 0, "Debug parameter for use by developers when creating new plasticity models, not for general use.  2 = debug Jacobian entries, 3 = check the entire Jacobian");
  params.addParam<RealTensorValue>("debug_jac_at_stress", RealTensorValue(), "Debug Jacobian entries at this stress.  For use by developers");
  params.addParam<std::vector<Real> >("debug_jac_at_pm", "Debug Jacobian entries at these plastic multipliers");
  params.addParam<std::vector<Real> >("debug_jac_at_intnl", "Debug Jacobian entries at these internal parameters");
  params.addParam<Real>("debug_stress_change", 1.0, "Debug finite differencing parameter for the stress");
  params.addParam<std::vector<Real> >("debug_pm_change", "Debug finite differencing parameters for the plastic multipliers");
  params.addParam<std::vector<Real> >("debug_intnl_change", "Debug finite differencing parameters for the internal parameters");
  params.addClassDescription("Base class for non-associative finite-strain plasticity");

  return params;
}

FiniteStrainPlasticBase::FiniteStrainPlasticBase(const InputParameters & parameters) :
    FiniteStrainMaterial(parameters),
    _max_iter(getParam<unsigned int>("max_NR_iterations")),
    _max_subdivisions(getParam<unsigned int>("max_subdivisions")),
    _f_tol(getParam<std::vector<Real> >("yield_function_tolerance")),
    _ic_tol(parameters.isParamValid("internal_constraint_tolerance") ? getParam<std::vector<Real> >("internal_constraint_tolerance") : std::vector<Real>(0)),
    _epp_tol(getParam<Real>("ep_plastic_tolerance")),

    _fspb_debug(getParam<int>("debug_fspb")),
    _fspb_debug_stress(getParam<RealTensorValue>("debug_jac_at_stress")),
    _fspb_debug_pm(getParam<std::vector<Real> >("debug_jac_at_pm")),
    _fspb_debug_intnl(getParam<std::vector<Real> >("debug_jac_at_intnl")),
    _fspb_debug_stress_change(getParam<Real>("debug_stress_change")),
    _fspb_debug_pm_change(getParam<std::vector<Real> >("debug_pm_change")),
    _fspb_debug_intnl_change(getParam<std::vector<Real> >("debug_intnl_change")),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _intnl(declareProperty<std::vector<Real> >("plastic_internal_parameter")),
    _intnl_old(declarePropertyOld<std::vector<Real> >("plastic_internal_parameter")),
    _f(declareProperty<std::vector<Real> >("plastic_yield_function")),
    _iter(declareProperty<unsigned int>("plastic_NR_iterations"))
{
}


void
FiniteStrainPlasticBase::initQpStatefulProperties()
{
  // Can't do these checks in the constructor because
  // this base class won't know about the derived class at that stage
  // so numberOfYieldFunctions() and numberOfInternalParameters()
  // won't be correctly overriden.
  if (_f_tol.size() != numberOfYieldFunctions())
    mooseError("The number of yield_function_tolerance parameters must match the number of yield functions");
  for (unsigned alpha = 0; alpha < _f_tol.size(); ++alpha)
    if (_f_tol[alpha] <= 0)
      mooseError("The yield_function_tolerance must be positive");

  if (_ic_tol.size() != numberOfInternalParameters())
    mooseError("The number of internal_constraint_tolerance parameters (" << _ic_tol.size() << ") must match the number of internal parameters (" << numberOfInternalParameters() << ")");
  for (unsigned a = 0; a < _ic_tol.size(); ++a)
    if (_ic_tol[a] <= 0)
      mooseError("The internal_constraint_tolerance must be positive");

  FiniteStrainMaterial::initQpStatefulProperties();

  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();

  _intnl[_qp].assign(numberOfInternalParameters(), 0);
  _intnl_old[_qp].assign(numberOfInternalParameters(), 0);

  _f[_qp].assign(numberOfYieldFunctions(), 0);

  _iter[_qp] = 0;

  if (_fspb_debug == 2)
  {
    checkDerivatives();
    mooseError("Derivatives have been checked.  Exiting with no error");
  }
}

void
FiniteStrainPlasticBase::computeQpStress()
{
  if (_fspb_debug == 3)
  {
    checkJacobian();
    mooseError("Jacobian has been checked.  Exiting with no error");
  }

  preReturnMap();

  /**
   * the idea in the following is to potentially subdivide the
   * strain increment into smaller portions
   * First one subdivision is tried, and if that fails then
   * 2 are tried, then 4, etc.  This is in the hope that the
   * time-step for the entire mesh need not be cut if there
   * are only a few "bad" quadpoints where the return-map
   * is difficult
   */

  bool return_successful = false;

  // number of subdivisions of the strain increment currently being tried
  unsigned int num_subdivisions = 1;

  while (num_subdivisions <= _max_subdivisions && !return_successful)
  {
    // prepare the variables for this set of strain increments
    RankTwoTensor dep = _strain_increment[_qp]/num_subdivisions;
    RankTwoTensor stress_previous = _stress_old[_qp];
    RankTwoTensor plastic_strain_previous = _plastic_strain_old[_qp];
    std::vector<Real> intnl_previous;
    intnl_previous.resize(_intnl_old[_qp].size());
    for (unsigned int a = 0; a < _intnl_old[_qp].size(); ++a)
      intnl_previous[a] = _intnl_old[_qp][a];


    // now do the work: apply the "dep" over num_subdivisions "substeps"
    for (unsigned substep = 0; substep < num_subdivisions; ++substep)
    {
      return_successful = returnMap(stress_previous, plastic_strain_previous, _intnl_old[_qp], dep, _elasticity_tensor[_qp], _stress[_qp], _plastic_strain[_qp], _intnl[_qp], _f[_qp], _iter[_qp]);
      if (return_successful)
      {
        // record the updated variables in readiness for the next substep
        stress_previous = _stress[_qp];
        plastic_strain_previous = _plastic_strain[_qp];
        for (unsigned int a = 0; a < _intnl_old[_qp].size(); ++a)
          intnl_previous[a] = _intnl[_qp][a];
      }
      else
        break; // oh dear, we need to increase the number of subdivisions
    }

    if (!return_successful)
      num_subdivisions *= 2;
  }

  if (!return_successful)
  {
    _console << "After making " << num_subdivisions << " subdivisions of the strain increment with L2norm " << _strain_increment[_qp].L2norm() << " the returnMap algorithm failed\n";
    _fspb_debug_stress = _stress_old[_qp];
    _fspb_debug_pm.assign(numberOfYieldFunctions(), 1); // this is chosen arbitrarily - please change if a more suitable value occurs to you!
    _fspb_debug_intnl.resize(numberOfInternalParameters());
    for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
      _fspb_debug_intnl[a] = _intnl_old[_qp][a];
    checkDerivatives();
    checkJacobian();
    mooseError("Exiting\n");
  }


  postReturnMap();

  //Update measures of strain
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp] - (_plastic_strain[_qp] - _plastic_strain_old[_qp]);
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  //Rotate the tensors to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();
  _elastic_strain[_qp] = _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
  _total_strain[_qp] = _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();
  _plastic_strain[_qp] = _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();
}

void
FiniteStrainPlasticBase::preReturnMap()
{
}

void
FiniteStrainPlasticBase::postReturnMap()
{
}

unsigned int
FiniteStrainPlasticBase::numberOfYieldFunctions()
{
  return 1; // one yield function
}

unsigned int
FiniteStrainPlasticBase::numberOfInternalParameters()
{
  return 0; // no internal parameters
}

void
FiniteStrainPlasticBase::yieldFunction(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<Real> & f)
{
  f.assign(1, 1.0); // one yield function, with the value 1.0
}

void
FiniteStrainPlasticBase::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(1, RankTwoTensor()); // one yield function, with all derivatives = 0 (the zeroes come from RankTwoTensor initialization)
}

void
FiniteStrainPlasticBase::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<Real> > & df_dintnl)
{
  df_dintnl.resize(1); // one yield function
  df_dintnl[0].resize(0); // zero internal parameters (ie, perfect plasticity)
}

void
FiniteStrainPlasticBase::flowPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & r)
{
  r.resize(1, RankTwoTensor()); // zero flow potential (zero comes from RankTwoTensor initialization)
}

void
FiniteStrainPlasticBase::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.resize(1, RankFourTensor()); // zero derivative (zero comes from RankFourTensor intialization)
}

void
FiniteStrainPlasticBase::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dr_dintnl)
{
  dr_dintnl.resize(0); // zero internal parameters (ie perfect plasticity)
}

void
FiniteStrainPlasticBase::hardPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<Real> > & h)
{
  h.resize(0); // zero internal parameters mean zero hardening potentials
}

void
FiniteStrainPlasticBase::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dh_dstress)
{
  dh_dstress.resize(0); // zero internal parameters mean zero hardening potentials
}

void
FiniteStrainPlasticBase::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<std::vector<Real> > > & dh_dintnl)
{
  dh_dintnl.resize(0); // zero internal parameters mean zero hardening potentials
}

bool
FiniteStrainPlasticBase::returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f, unsigned int & iter)
{

  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  stress = stress_old + E_ijkl * delta_d; // the trial stress
  plastic_strain = plastic_strain_old;
  for (unsigned i = 0; i < intnl_old.size(); ++i)
    intnl[i] = intnl_old[i];
  iter = 0;

  yieldFunction(stress, intnl, f);

  Real nr_res2 = 0;
  for (unsigned i = 0; i < f.size(); ++i)
    nr_res2 += 0.5*std::pow( std::max(f[i], 0.0)/_f_tol[i], 2);

  if (nr_res2 < 0.5)
    // a purely elastic increment.
    // All output variables have been calculated
    return true;


  // So, from here on we know that the trial stress
  // is inadmissible, and we have to return from that
  // value to the yield surface.  There are three
  // types of constraints we have to satisfy, listed
  // below, and calculated in calculateConstraints(...)

  // Plastic strain constraint, L2 norm must be zero (up to a tolerance)
  RankTwoTensor epp;

  // Yield function constraint passed to this function as
  // std::vector<Real> & f
  // Each yield function must be <= 0 (up to tolerance)

  // Internal constraint(s), must be zero (up to a tolerance)
  std::vector<Real> ic;


  // During the Newton-Raphson procedure, we'll be
  // changing the following parameters in order to
  // (attempt to) satisfy the constraints.
  RankTwoTensor dstress; // change in stress
  std::vector<Real> dpm; // change in plasticity multipliers ("consistency parameters")
  std::vector<Real>  dintnl; // change in internal parameters



  // The following are used in the Newton-Raphson

  // Inverse of E_ijkl (assuming symmetric)
  RankFourTensor E_inv = E_ijkl.invSymm();

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(trial_stress - stress), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp;

  // The "consistency parameters" (plastic multipliers)
  // Change in plastic strain in this timestep = pm*flowPotential
  // Each pm must be non-negative
  std::vector<Real> pm;
  pm.assign(numberOfYieldFunctions(), 0.0);

  // whether line-searching was successful
  bool ls_success = true;

  // The Newton-Raphson loops
  while (nr_res2 > 0.5 && iter < _max_iter && ls_success)
  {
    iter++;

    // calculate dstress, dpm and dintnl for one full Newton-Raphson step
    nrStep(stress, intnl_old, intnl, pm, E_inv, delta_dp, dstress, dpm, dintnl);

    // perform a line search
    // The line-search will exit with updated values
    ls_success = lineSearch(nr_res2, stress, intnl_old, intnl, pm, E_inv, delta_dp, dstress, dpm, dintnl, f, epp, ic);
  }


  if (iter >= _max_iter || !ls_success)
  {
    stress = stress_old;
    for (unsigned i = 0; i < intnl_old.size(); ++i)
      intnl[i] = intnl_old[i];
    return false;
  }
  else
  {
    plastic_strain += delta_dp;
    return true;
  }

}

void
FiniteStrainPlasticBase::calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic)
{
  // yield functions
  yieldFunction(stress, intnl, f);


  // flow direction
  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, r);

  epp = RankTwoTensor();
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    epp += pm[alpha]*r[alpha];
  epp -= delta_dp;


  // internal constraints
  std::vector<std::vector<Real> > h;
  hardPotential(stress, intnl, h);

  ic.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    ic[a] = intnl[a] - intnl_old[a];
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      ic[a] += pm[alpha]*h[a][alpha];
  }
}

void
FiniteStrainPlasticBase::calculateRHS(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & rhs)
{
  std::vector<Real> f; // the yield functions
  RankTwoTensor epp; // the plastic-strain constraint ("direction constraint")
  std::vector<Real> ic; // the "internal constraints"

  calculateConstraints(stress, intnl_old, intnl, pm, delta_dp, f, epp, ic);

  unsigned int dim = 3;
  unsigned int system_size = 6 + numberOfYieldFunctions() + numberOfInternalParameters(); // "6" comes from symmeterizing epp

  rhs.resize(system_size);

  // rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[numberOfYieldFunctions()], ic[0], ic[1], ..., ic[numberOfInternalParameters()])
  // notice the appearance of only the i>=j components

  unsigned ind = 0;
  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
      rhs[ind++] = -epp(i, j);
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    rhs[ind++] = -f[alpha];
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    rhs[ind++] = -ic[a];
}


Real
FiniteStrainPlasticBase::residual2(const std::vector<Real> & f, const RankTwoTensor & epp, const std::vector<Real> & ic)
{
  Real nr_res2 = 0;
  for (unsigned alpha = 0; alpha < f.size(); ++alpha)
    nr_res2 += 0.5*std::pow( f[alpha]/_f_tol[alpha], 2); // NOTE: have to change for multi-surface, since f could be negative!
  nr_res2 += 0.5*std::pow(epp.L2norm()/_epp_tol, 2);
  for (unsigned a = 0; a < ic.size(); ++a)
    nr_res2 += 0.5*std::pow(ic[a]/_ic_tol[a], 2);
  return nr_res2;
}


void
FiniteStrainPlasticBase::calculateJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac)
{
  // construct quantities used in the Newton-Raphson linear system
  // These should have been defined by the derived classes, if
  // they are different from the default functions given elsewhere
  // in this class
  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, df_dstress);

  std::vector<std::vector<Real> > df_dintnl;
  dyieldFunction_dintnl(stress, intnl, df_dintnl);

  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, r);

  std::vector<RankFourTensor> dr_dstress;
  dflowPotential_dstress(stress, intnl, dr_dstress);

  std::vector<std::vector<RankTwoTensor> > dr_dintnl;
  dflowPotential_dintnl(stress, intnl, dr_dintnl);

  std::vector<std::vector<Real> > h;
  hardPotential(stress, intnl, h);

  std::vector<std::vector<RankTwoTensor> > dh_dstress;
  dhardPotential_dstress(stress, intnl, dh_dstress);

  std::vector<std::vector<std::vector<Real> > > dh_dintnl;
  dhardPotential_dintnl(stress, intnl, dh_dintnl);


  // construct matrix entries
  // In the following
  // epp = pm*r - E_inv*(trial_stress - stress) = pm*r - delta_dp
  // f = yield function
  // ic = intnl - intnl_old + pm*h

  RankFourTensor depp_dstress;
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    depp_dstress += pm[alpha]*dr_dstress[alpha];
  depp_dstress += E_inv;

  std::vector<RankTwoTensor> depp_dpm;
  depp_dpm.resize(numberOfYieldFunctions());
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    depp_dpm[alpha] = r[alpha];

  std::vector<RankTwoTensor> depp_dintnl;
  depp_dintnl.assign(numberOfInternalParameters(), RankTwoTensor());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      depp_dintnl[a] += pm[alpha]*dr_dintnl[alpha][a];

  // df_dstress has been calculated above
  // df_dpm is always zero
  // df_dintnl has been calculated above

  std::vector<RankTwoTensor> dic_dstress;
  dic_dstress.assign(numberOfInternalParameters(), RankTwoTensor());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      dic_dstress[a] += pm[alpha]*dh_dstress[a][alpha];

  std::vector<std::vector<Real> > dic_dpm;
  dic_dpm.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    dic_dpm[a].resize(numberOfYieldFunctions());
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      dic_dpm[a][alpha] = h[a][alpha];
  }

  std::vector<std::vector<Real> > dic_dintnl;
  dic_dintnl.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    dic_dintnl[a].assign(numberOfInternalParameters(), 0);
    for (unsigned b = 0; b < numberOfInternalParameters(); ++b)
      for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
        dic_dintnl[a][b] += pm[alpha]*dh_dintnl[a][alpha][b];
    dic_dintnl[a][a] += 1;
  }


  /**
   * now construct the Jacobian
   * It is:
   * ( depp_dstress depp_dpm depp_dintnl )
   * (  df_dstress       0      df_dintnl   )
   * ( dic_dstress    dic_dpm   dic_dintnl  )
   * For the "epp" terms, only the i>=j components are kept in the RHS, so only these terms are kept here too
   */

  unsigned int dim = 3;
  unsigned int system_size = 6 + numberOfYieldFunctions() + numberOfInternalParameters(); // "6" comes from symmeterizing epp
  jac.resize(system_size);
  for (unsigned i = 0; i < system_size; ++i)
    jac[i].resize(system_size);

  unsigned int row_num = 0;
  unsigned int col_num = 0;
  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
    {
      for (unsigned k = 0; k < dim; ++k)
        for (unsigned l = 0; l <= k; ++l)
          jac[col_num][row_num++] = depp_dstress(i, j, k, l) + (k != l ? depp_dstress(i, j, l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
      for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
        jac[col_num][row_num++] = depp_dpm[alpha](i, j);
      for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
        jac[col_num][row_num++] = depp_dintnl[a](i, j);
      row_num = 0;
      col_num++;
    }

  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
  {
    for (unsigned k = 0; k < dim; ++k)
      for (unsigned l = 0; l <= k; ++l)
        jac[col_num][row_num++] = df_dstress[alpha](k, l) + (k != l ? df_dstress[alpha](l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
    for (unsigned beta = 0; beta < numberOfYieldFunctions(); ++beta)
      jac[col_num][row_num++] = 0;
    for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
      jac[col_num][row_num++] = df_dintnl[alpha][a];
    row_num = 0;
    col_num++;
  }

  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    for (unsigned k = 0; k < dim; ++k)
      for (unsigned l = 0; l <= k; ++l)
        jac[col_num][row_num++] = dic_dstress[a](k, l) + (k != l ? dic_dstress[a](l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      jac[col_num][row_num++] = dic_dpm[a][alpha];
    for (unsigned b = 0; b < numberOfInternalParameters(); ++b)
      jac[col_num][row_num++] = dic_dintnl[a][b];
    row_num = 0;
    col_num++;
  }
}

void
FiniteStrainPlasticBase::nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl)
{
  // Calculate RHS and Jacobian
  std::vector<Real> rhs;
  calculateRHS(stress, intnl_old, intnl, pm, delta_dp, rhs);

  std::vector<std::vector<Real> > jac;
  calculateJacobian(stress, intnl, pm, E_inv, jac);

  // prepare for LAPACKgesv_ routine provided by PETSc (at least since PETSc 3.0.0)
  int system_size = rhs.size();

  std::vector<double> a(system_size*system_size);
  // Fill in the a "matrix" by going down columns
  unsigned ind = 0;
  for (int col = 0; col < system_size; ++col)
    for (int row = 0; row < system_size; ++row)
      a[ind++] = jac[row][col];

  int nrhs = 1;
  std::vector<int> ipiv(system_size);
  int info;
  LAPACKgesv_(&system_size, &nrhs, &a[0], &system_size, &ipiv[0], &rhs[0], &system_size, &info);

  if (info != 0)
    mooseError("In solving the linear system in a Newton-Raphson process, the PETSC LAPACK gsev routine returned with error code " << info);

  // Extract the results back to dstress, dpm and dintnl
  unsigned int dim = 3;
  ind = 0;
  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
      dstress(i, j) = dstress(j, i) = rhs[ind++];
  dpm.resize(numberOfYieldFunctions());
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    dpm[alpha] = rhs[ind++];
  dintnl.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    dintnl[a] = rhs[ind++];
}


bool
FiniteStrainPlasticBase::lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic)
{
  // Line search algorithm straight out of "Numerical Recipes"

  bool success = true; // return value: will be false if linesearch couldn't reduce the residual-squared

  // Aim is to decrease residual2

  Real lam = 1.0; // the line-search parameter: 1.0 is a full Newton step
  Real lam_min = 1E-10; // minimum value of lam allowed - perhaps this should be dynamically calculated?
  Real f0 = nr_res2; // initial value of residual2
  Real slope = -2*nr_res2; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but i hope the nrStep would warn if there were problems.
  Real tmp_lam; // cached value of lam used in quadratic & cubic line search
  Real f2 = nr_res2; // cached value of f = residual2 used in the cubic in the line search
  Real lam2 = lam; // cached value of lam used in the cubic in the line search


  // pm during the line-search
  std::vector<Real> ls_pm;
  ls_pm.resize(numberOfYieldFunctions());

  // delta_dp during the line-search
  RankTwoTensor ls_delta_dp;

  // internal parameter during the line-search
  std::vector<Real> ls_intnl;
  ls_intnl.resize(numberOfInternalParameters());

  // stress during the line-search
  RankTwoTensor ls_stress;


  while (true)
  {
    // update the variables using this line-search parameter
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      ls_pm[alpha] = pm[alpha] + dpm[alpha]*lam;
    ls_delta_dp = delta_dp - E_inv*dstress*lam;
    for (unsigned a = 0; a < numberOfInternalParameters(); ++ a)
      ls_intnl[a] = intnl[a] + dintnl[a]*lam;
    ls_stress = stress + dstress*lam;

    // calculate the new yield function, epp and internal constraints
    calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, epp, ic);

    // calculate the new residual-squared
    nr_res2 = residual2(f, epp, ic);


    if (nr_res2 < f0 + 1E-4*lam*slope)
      break;
    else if (lam < lam_min)
    {
      success = false;
      // restore plastic multipliers, yield functions, etc to original values
      for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
        ls_pm[alpha] = pm[alpha];
      ls_delta_dp = delta_dp;
      for (unsigned a = 0; a < numberOfInternalParameters(); ++ a)
        ls_intnl[a] = intnl[a];
      ls_stress = stress;
      calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, epp, ic);
      nr_res2 = residual2(f, epp, ic);
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
  }

  // assign the quantities found in the line-search
  // back to the originals
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    pm[alpha] = ls_pm[alpha];
  delta_dp = ls_delta_dp;
  for (unsigned a = 0; a < numberOfInternalParameters(); ++ a)
    intnl[a] = ls_intnl[a];
  stress = ls_stress;

  return success;
}



// ********************************************
// *                                          *
// *  FINITE DIFFERENCE CHECKING ROUTINES     *
// *                                          *
// ********************************************

void
FiniteStrainPlasticBase::checkDerivatives()
{
  _console << "\n ++++++++++++++ \nChecking the derivatives\n";
  outputAndCheckDebugParameters();

  _console << "dyieldFunction_dstress.  Relative L2 norms.\n";
  std::vector<RankTwoTensor> df_dstress;
  std::vector<RankTwoTensor> fddf_dstress;
  dyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, df_dstress);
  fddyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddf_dstress);
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
  {
    _console << "alpha = " << alpha << " Relative L2norm = " << 2*(df_dstress[alpha] - fddf_dstress[alpha]).L2norm()/(df_dstress[alpha] + fddf_dstress[alpha]).L2norm() << "\n";
    _console << "Coded:\n";
    df_dstress[alpha].print();
    _console << "Finite difference:\n";
    fddf_dstress[alpha].print();
  }

  _console << "dflowPotential_dstress.  Relative L2 norms.\n";
  std::vector<RankFourTensor> dr_dstress;
  std::vector<RankFourTensor> fddr_dstress;
  dflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, dr_dstress);
  fddflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddr_dstress);
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
  {
    _console << "alpha = " << alpha << " Relative L2norm = " << 2*(dr_dstress[alpha] - fddr_dstress[alpha]).L2norm()/(dr_dstress[alpha] + fddr_dstress[alpha]).L2norm() << "\n";
    _console << "Coded:\n";
    dr_dstress[alpha].print();
    _console << "Finite difference:\n";
    fddr_dstress[alpha].print();
  }

  _console << "dflowPotential_dintnl.  Relative L2 norms.\n";
  std::vector<std::vector<RankTwoTensor> > dr_dintnl;
  std::vector<std::vector<RankTwoTensor> > fddr_dintnl;
  dflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, dr_dintnl);
  fddflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, fddr_dintnl);
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
  {
    for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    {
      _console << "alpha = " << alpha << " a = " << a << " Relative L2norm = " << 2*(dr_dintnl[alpha][a] - fddr_dintnl[alpha][a]).L2norm()/(dr_dintnl[alpha][a] + fddr_dintnl[alpha][a]).L2norm() << "\n";
      _console << "Coded:\n";
      dr_dintnl[alpha][a].print();
      _console << "Finite difference:\n";
      fddr_dintnl[alpha][a].print();
    }
  }

}

void
FiniteStrainPlasticBase::fddyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(numberOfYieldFunctions(), RankTwoTensor());

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<Real> fep, fep_minus;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      stressep = stress;
      // do a central difference to attempt to capture discontinuities
      // such as those encountered in tensile and Mohr-Coulomb
      stressep(i, j) += ep/2.0;
      yieldFunction(stressep, intnl, fep);
      stressep(i, j) -= ep;
      yieldFunction(stressep, intnl, fep_minus);
      for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
        df_dstress[alpha](i, j) = (fep[alpha] - fep_minus[alpha])/ep;
    }
}

void
FiniteStrainPlasticBase::fddflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.assign(numberOfYieldFunctions(), RankFourTensor());

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<RankTwoTensor> rep, rep_minus;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      stressep = stress;
      stressep(i, j) += ep/2.0;
      flowPotential(stressep, intnl, rep);
      stressep(i, j) -= ep;
      flowPotential(stressep, intnl, rep_minus);
      for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
            dr_dstress[alpha](k, l, i, j) = (rep[alpha](k, l) - rep_minus[alpha](k, l))/ep;
    }
}

void
FiniteStrainPlasticBase::fddflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dr_dintnl)
{
  dr_dintnl.resize(numberOfYieldFunctions());
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    dr_dintnl[alpha].assign(numberOfInternalParameters(), RankTwoTensor());

  std::vector<RankTwoTensor> origr;
  flowPotential(stress, intnl, origr);

  std::vector<Real> intnlep;
  intnlep.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    intnlep[a] = intnl[a];
  Real ep;
  std::vector<RankTwoTensor> rep;
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    ep = _fspb_debug_intnl_change[a];
    intnlep[a] += ep;
    flowPotential(stress, intnlep, rep);
    for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
      dr_dintnl[alpha][a] = (rep[alpha] - origr[alpha])/ep;
    intnlep[a] -= ep;
  }
}


void
FiniteStrainPlasticBase::checkJacobian()
{
  _console << "\n ++++++++++++++ \nChecking the Jacobian\n";
  outputAndCheckDebugParameters();

  RankFourTensor E_inv = _elasticity_tensor[_qp].invSymm();
  RankTwoTensor delta_dp = -E_inv*_fspb_debug_stress;

  std::vector<std::vector<Real> > jac;
  calculateJacobian(_fspb_debug_stress, _fspb_debug_intnl, _fspb_debug_pm, E_inv, jac);

  std::vector<std::vector<Real> > fdjac;
  fdJacobian(_fspb_debug_stress, _intnl_old[_qp], _fspb_debug_intnl, _fspb_debug_pm, delta_dp, E_inv, fdjac);

  _console << "Hand-coded Jacobian:\n";
  for (unsigned row = 0; row < jac.size(); ++row)
  {
    for (unsigned col = 0; col < jac.size(); ++col)
      _console << jac[row][col] << " ";
    _console << "\n";
  }

  _console << "Finite difference Jacobian:\n";
  for (unsigned row = 0; row < fdjac.size(); ++row)
  {
    for (unsigned col = 0; col < fdjac.size(); ++col)
      _console << fdjac[row][col] << " ";
    _console << "\n";
  }
}

void
FiniteStrainPlasticBase::fdJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac)
{
  std::vector<Real> orig_rhs;
  calculateRHS(stress, intnl_old, intnl, pm, delta_dp, orig_rhs);

  unsigned int system_size = orig_rhs.size();
  jac.resize(system_size);
  for (unsigned row = 0; row < system_size; ++row)
    jac[row].resize(system_size);


  std::vector<Real> rhs_ep;
  unsigned col = 0;

  RankTwoTensor stressep;
  RankTwoTensor delta_dpep;
  Real ep = _fspb_debug_stress_change;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j <= i; ++j)
    {
      stressep = stress;
      stressep(i, j) += ep;
      if (i != j)
        stressep(j, i) += ep;
      delta_dpep = delta_dp;
      for (unsigned k = 0; k < 3; ++k)
        for (unsigned l = 0; l < 3; ++l)
        {
          delta_dpep(k, l) -= E_inv(k, l, i, j)*ep;
          if (i != j)
            delta_dpep(k, l) -= E_inv(k, l, j, i)*ep;
        }
      calculateRHS(stressep, intnl_old, intnl, pm, delta_dpep, rhs_ep);
      for (unsigned row = 0; row < system_size; ++row)
        jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
      col++;
    }

  std::vector<Real> pmep;
  pmep.resize(numberOfYieldFunctions());
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    pmep[alpha] = pm[alpha];
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
  {
    ep = _fspb_debug_pm_change[alpha];
    pmep[alpha] += ep;
    calculateRHS(stress, intnl_old, intnl, pmep, delta_dp, rhs_ep);
    for (unsigned row = 0; row < system_size; ++row)
      jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
    pmep[alpha] -= ep;
    col++;
  }

  std::vector<Real> intnlep;
  intnlep.resize(numberOfInternalParameters());
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    intnlep[a] = intnl[a];
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
  {
    ep = _fspb_debug_intnl_change[a];
    intnlep[a] += ep;
    calculateRHS(stress, intnl_old, intnlep, pm, delta_dp, rhs_ep);
    for (unsigned row = 0; row < system_size; ++row)
      jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
    intnlep[a] -= ep;
    col++;
  }
}


void
FiniteStrainPlasticBase::outputAndCheckDebugParameters()
{
  _console << "stress = \n";
  _fspb_debug_stress.print();

  if (_fspb_debug_pm.size() != numberOfYieldFunctions() || _fspb_debug_intnl.size() != numberOfInternalParameters() || _fspb_debug_pm_change.size() != numberOfYieldFunctions() || _fspb_debug_intnl_change.size() != numberOfInternalParameters())
    mooseError("The debug parameters have the wrong size\n");

  _console << "plastic multipliers =\n";
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    _console << _fspb_debug_pm[alpha] << "\n";

  _console << "internal parameters =\n";
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    _console << _fspb_debug_intnl[a] << "\n";

  _console << "finite-differencing parameter for stress-changes:\n" << _fspb_debug_stress_change  << "\n";
  _console << "finite-differencing parameter(s) for plastic-multiplier(s):\n";
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions(); ++alpha)
    _console << _fspb_debug_pm_change[alpha] << "\n";
  _console << "finite-differencing parameter(s) for internal-parameter(s):\n";
  for (unsigned a = 0; a < numberOfInternalParameters(); ++a)
    _console << _fspb_debug_intnl_change[a] << "\n";
}

