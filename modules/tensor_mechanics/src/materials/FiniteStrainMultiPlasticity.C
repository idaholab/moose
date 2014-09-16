#include "FiniteStrainMultiPlasticity.h"

// Following is for perturbing distances in eliminating linearly-dependent directions
#include "MooseRandom.h"

// Following is used to access PETSc's LAPACK routines
#include <petscblaslapack.h>

template<>
InputParameters validParams<FiniteStrainMultiPlasticity>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRangeCheckedParam<unsigned int>("max_NR_iterations", 20, "max_NR_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addRequiredParam<std::vector<UserObjectName> >("plastic_models", "List of names of user objects that define the plastic models that could be active for this material.");
  params.addRequiredRangeCheckedParam<Real>("ep_plastic_tolerance", "ep_plastic_tolerance>0", "The Newton-Raphson process is only deemed converged if the plastic strain increment constraints have L2 norm less than this.");
  params.addRangeCheckedParam<unsigned int>("max_subdivisions", 64, "max_subdivisions>0", "If ordinary Newton-Raphson + line-search fails, then the applied strain increment is subdivided, and the return-map is tried again.  This parameter is the maximum number of subdivisions allowed.  The number of subdivisions tried increases exponentially, first 1, then 2, then 4, then 8, etc");
  params.addRangeCheckedParam<Real>("linear_dependent", 1E-4, "linear_dependent>=0 & linear_dependent<1", "Flow directions are considered linearly dependent if the smallest singular value is less than linear_dependent times the largest singular value");
  MooseEnum deactivation_scheme("optimized safe optimized_to_safe", "optimized");
  params.addParam<MooseEnum>("deactivation_scheme", deactivation_scheme, "Scheme by which constraints are deactivated.  safe: return to the yield surface and then deactivate constraints with negative plasticity multipliers.  optimized: deactivate a constraint as soon as its plasticity multiplier becomes negative.  optimized_to_safe: first use 'optimized', and if that fails, try the return with 'safe' instead.");
  params.addParam<int>("debug_fspb", 0, "Debug parameter for use by developers when creating new plasticity models, not for general use.  2 = debug Jacobian entries, 3 = check the entire Jacobian");
  params.addParam<RealTensorValue>("debug_jac_at_stress", RealTensorValue(), "Debug Jacobian entries at this stress.  For use by developers");
  params.addParam<std::vector<Real> >("debug_jac_at_pm", "Debug Jacobian entries at these plastic multipliers");
  params.addParam<std::vector<Real> >("debug_jac_at_intnl", "Debug Jacobian entries at these internal parameters");
  params.addParam<Real>("debug_stress_change", 1.0, "Debug finite differencing parameter for the stress");
  params.addParam<std::vector<Real> >("debug_pm_change", "Debug finite differencing parameters for the plastic multipliers");
  params.addParam<std::vector<Real> >("debug_intnl_change", "Debug finite differencing parameters for the internal parameters");
  params.addClassDescription("Base class for multi-surface finite-strain plasticity");

  return params;
}

FiniteStrainMultiPlasticity::FiniteStrainMultiPlasticity(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
    _max_iter(getParam<unsigned int>("max_NR_iterations")),
    _max_subdivisions(getParam<unsigned int>("max_subdivisions")),

    _num_f(getParam<std::vector<UserObjectName> >("plastic_models").size()),
    _epp_tol(getParam<Real>("ep_plastic_tolerance")),

    _svd_tol(getParam<Real>("linear_dependent")),
    _min_f_tol(-1.0),

    _deactivation_scheme(getParam<MooseEnum>("deactivation_scheme")),

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
    _yf(declareProperty<std::vector<Real> >("plastic_yield_function")),
    _iter(declareProperty<Real>("plastic_NR_iterations")) // this is really an unsigned int, but for visualisation i convert it to Real
{
  _f.resize(_num_f);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    _f[alpha] = &getUserObjectByName<TensorMechanicsPlasticModel>(getParam<std::vector<UserObjectName> >("plastic_models")[alpha]);
    if (_min_f_tol == -1.0 || _min_f_tol > _f[alpha]->_f_tol)
      _min_f_tol = _f[alpha]->_f_tol;
  }
  MooseRandom::seed(0);
}


void
FiniteStrainMultiPlasticity::initQpStatefulProperties()
{
  FiniteStrainMaterial::initQpStatefulProperties();

  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();

  _intnl[_qp].assign(_num_f, 0);
  _intnl_old[_qp].assign(_num_f, 0);

  _yf[_qp].assign(_num_f, 0);

  _iter[_qp] = 0.0; // this is really an unsigned int, but for visualisation i convert it to Real

  if (_fspb_debug == 2)
  {
    checkDerivatives();
    mooseError("Derivatives have been checked.  Exiting with no error");
  }

}

void
FiniteStrainMultiPlasticity::computeQpStress()
{
  if (_fspb_debug == 3)
  {
    checkJacobian();
    mooseError("Jacobian has been checked.  Exiting with no error");
  }


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

  // number of iterations in the current return map
  unsigned int iter;

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
    _iter[_qp] = 0.0;


    // now do the work: apply the "dep" over num_subdivisions "substeps"
    for (unsigned substep = 0 ; substep < num_subdivisions ; ++substep)
    {
      return_successful = returnMap(stress_previous, plastic_strain_previous, _intnl_old[_qp], dep, _elasticity_tensor[_qp], _stress[_qp], _plastic_strain[_qp], _intnl[_qp], _yf[_qp], iter);
      if (return_successful)
      {
        // record the updated variables in readiness for the next substep
        stress_previous = _stress[_qp];
        plastic_strain_previous = _plastic_strain[_qp];
        for (unsigned int a = 0; a < _intnl_old[_qp].size(); ++a)
          intnl_previous[a] = _intnl[_qp][a];
        _iter[_qp] += 1.0*iter;
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
    _fspb_debug_pm.assign(_num_f, 1); // this is chosen arbitrarily - please change if a more suitable value occurs to you!
    _fspb_debug_intnl.resize(_num_f);
    for (unsigned a = 0 ; a < _num_f ; ++a)
      _fspb_debug_intnl[a] = _intnl_old[_qp][a];
    mooseError("Exiting\n");
    checkDerivatives();
    checkJacobian();
  }


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
FiniteStrainMultiPlasticity::yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & f)
{
  f.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      f[ind++] = _f[alpha]->yieldFunction(stress, intnl[alpha]);
}

void
FiniteStrainMultiPlasticity::dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      df_dstress[ind++] = _f[alpha]->dyieldFunction_dstress(stress, intnl[alpha]);
}

void
FiniteStrainMultiPlasticity::dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & df_dintnl)
{
  df_dintnl.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      df_dintnl[ind++] = _f[alpha]->dyieldFunction_dintnl(stress, intnl[alpha]); // only diagonal terms by assumption
}

void
FiniteStrainMultiPlasticity::flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & r)
{
  r.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      r[ind++] = _f[alpha]->flowPotential(stress, intnl[alpha]);
}

void
FiniteStrainMultiPlasticity::dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      dr_dstress[ind++] = _f[alpha]->dflowPotential_dstress(stress, intnl[alpha]);
}

void
FiniteStrainMultiPlasticity::dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & dr_dintnl)
{
  dr_dintnl.resize(num_active);
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      dr_dintnl[ind++] = _f[alpha]->dflowPotential_dintnl(stress, intnl[alpha]);
}

void
FiniteStrainMultiPlasticity::hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & h)
{
  h.resize(num_active);
  unsigned ind = 0;
  for (unsigned a = 0 ; a < _num_f ; ++a)
    if (active[a])
      h[ind++] = _f[a]->hardPotential(stress, intnl[a]); // only diagonal terms by assumption
}

void
FiniteStrainMultiPlasticity::dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & dh_dstress)
{
  dh_dstress.resize(num_active);
  unsigned ind = 0;
  for (unsigned a = 0 ; a < _num_f ; ++a)
    if (active[a])
      dh_dstress[ind++] = _f[a]->dhardPotential_dstress(stress, intnl[a]);
}

void
FiniteStrainMultiPlasticity::dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & dh_dintnl)
{
  dh_dintnl.resize(num_active);
  unsigned ind = 0;
  for (unsigned a = 0 ; a < _num_f ; ++a)
    if (active[a])
      dh_dintnl[ind++] = _f[a]->dhardPotential_dintnl(stress, intnl[a]);
}

bool
FiniteStrainMultiPlasticity::returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f, unsigned int & iter)
{
  bool successful_return = true;  // return value of this function

  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  stress = stress_old + E_ijkl * delta_d; // the trial stress
  plastic_strain = plastic_strain_old;
  for (unsigned i = 0; i < intnl_old.size() ; ++i)
    intnl[i] = intnl_old[i];
  iter = 0;

  // active constraints.  At this stage assume they're all active
  std::vector<bool> act;
  act.assign(_num_f, true);

  yieldFunction(stress, intnl, act, _num_f, f);

  Real nr_res2 = 0;
  for (unsigned i = 0 ; i < f.size() ; ++i)
    if (f[i] > 0.0)
      nr_res2 += 0.5*std::pow(f[i]/_f[i]->_f_tol, 2);

  if (nr_res2 < 0.5)
    // a purely elastic increment.
    // All output variables have been calculated
    return successful_return;



  // So, from here on we know that the trial stress and intnl_old
  // is inadmissible, and we have to return from those values
  // value to the yield surface.  There are three
  // types of constraints we have to satisfy, listed
  // below, and calculated in calculateConstraints(...)
  //
  // Kuhn-Tucker conditions must also be satisfied
  // These are:
  // f<=0, epp=0, ic=0 (up to tolerances), and these are
  //     guaranteed to hold if nr_res2<=0.5
  // pm>=0, which may not hold upon exit of the NR loops
  //     due to _deactivation_scheme!="optimized";
  // pm*f=0 (up to tolerances), which may not hold upon exit
  //     of the NR loops if a constraint got deactivated
  //     due to linear dependence, and then f<0, and its pm>0


  // Plastic strain constraint, L2 norm must be zero (up to a tolerance)
  RankTwoTensor epp;

  // Yield function constraint passed to this function as
  // std::vector<Real> & f
  // Each yield function must be <= 0 (up to tolerance)
  // Note that only the constraints that are both active and not
  // deactivated due to linear dependence will be contained in f
  // until the final few lines of returnMap

  // Internal constraint(s), must be zero (up to a tolerance)
  // Note that only the constraints that are active and linearly
  // independent will be contained in ic.
  std::vector<Real> ic;


  // Inverse of E_ijkl (assuming symmetric)
  RankFourTensor E_inv = E_ijkl.invSymm();

  // Initialise the set of active constraints
  // At this stage, the active constraints are
  // those that exceed their _f_tol
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    act[alpha] = (f[alpha] > _f[alpha]->_f_tol);

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(trial_stress - stress), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp = RankTwoTensor();

  // The "consistency parameters" (plastic multipliers)
  // Change in plastic strain in this timestep = pm*flowPotential
  // Each pm must be non-negative
  std::vector<Real> pm;
  pm.assign(_num_f, 0.0);

  // whether single step was successful (whether line search was successful, and whether turning off constraints was successful)
  bool single_step_success = true;

  // deactivation scheme
  MooseEnum deact_scheme = _deactivation_scheme;

  // For complicated deactivation schemes we have to record the initial active set
  std::vector<bool> initial_act;
  initial_act.resize(_num_f);
  if (_deactivation_scheme == "optimized_to_safe")
  {
    // if "optimized" failes can change the deactivation scheme to "safe"
    deact_scheme = "optimized";
    for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
      initial_act[alpha] = act[alpha];
  }

      Moose::out << "Initial act = ";
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        Moose::out << act[alpha] << " ";
      Moose::out << "\n";


  successful_return = false;

  while (!successful_return)
  {
    single_step_success = true;

    iter = 0;

    // The Newton-Raphson loops
    while (nr_res2 > 0.5 && iter++ < _max_iter && single_step_success)
    {
      single_step_success = singleStep(nr_res2, stress, intnl_old, intnl, pm, delta_dp, E_inv, f, epp, ic, act, deact_scheme);
      Moose::out << "After singlestep stress_zz = " << stress(2, 2) << " stress_yy = " << stress(1, 1) << "\n";
      Moose::out << "and act = ";
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        Moose::out << act[alpha] << " ";
      Moose::out << "\n";
    }



    if (nr_res2 <= 0.5 && iter <= _max_iter && single_step_success)
    {
      // Returned, but must still check Kuhn-Tucker conditions
      if (checkAndApplyKuhnTucker(f, pm, act))
        successful_return = true; // Returned perfectly successfully.  The "while" loop will now exit
      else
        successful_return = false;
    }
    else if (deact_scheme == "optimized" && _deactivation_scheme == "optimized_to_safe")
    {
      // did not return successfully, but can try the "safe" version
      successful_return = false;
      deact_scheme = "safe";
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        act[alpha] = initial_act[alpha];
    }
    else
      // did not return, and cannot do anything about it
      break;


    if (!successful_return)
    {
      stress = stress_old + E_ijkl * delta_d; // back to trial stress
      delta_dp = RankTwoTensor(); // back to zero change in plastic strain
      for (unsigned i = 0; i < intnl_old.size() ; ++i)
        intnl[i] = intnl_old[i];  // back to old internal params
      pm.assign(_num_f, 0.0); // back to zero plastic multipliers

      unsigned num_active = numberActive(act);
      if (num_active == 0)
        break; // failure

      // calculate new nr_res2
      yieldFunction(stress, intnl, act, num_active, f);

      nr_res2 = 0;
      for (unsigned i = 0 ; i < num_active ; ++i)
        if (f[i] > 0.0)
          nr_res2 += 0.5*std::pow(f[i]/_f[i]->_f_tol, 2);
    }

  }

  // returned, with either success or failure
  if (!successful_return)
  {
    // FAILURE
    stress = stress_old;
    for (unsigned i = 0; i < intnl_old.size() ; ++i)
      intnl[i] = intnl_old[i];
    return false;
  }
  else
  {
    plastic_strain += delta_dp;
    // at this stage f.size() = num_active, but we need to return with all the yield functions evaluated, so:
    act.assign(_num_f, true);
    yieldFunction(stress, intnl, act, _num_f, f);
    return true;
  }

}

bool
FiniteStrainMultiPlasticity::singleStep(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<Real> & f,RankTwoTensor & epp, std::vector<Real> & ic, std::vector<bool> & active, const MooseEnum & deactivation_scheme)
{
  bool successful_step;  // return value

  Real nr_res2_before_step = nr_res2;
  RankTwoTensor stress_before_step;
  std::vector<Real> intnl_before_step;
  std::vector<Real> pm_before_step;
  RankTwoTensor delta_dp_before_step;

  if (deactivation_scheme == "optimized")
  {
    // we potentially used the "before_step" quantities, so record them here
    stress_before_step = stress;
    intnl_before_step.resize(intnl.size());
    for (unsigned alpha = 0 ; alpha < intnl.size() ; ++alpha)
      intnl_before_step[alpha] = intnl[alpha];
    pm_before_step.resize(pm.size());
    for (unsigned alpha = 0 ; alpha < pm.size() ; ++alpha)
      pm_before_step[alpha] = pm[alpha];
    delta_dp_before_step = delta_dp;
  }

  // During the Newton-Raphson procedure, we'll be
  // changing the following parameters in order to
  // (attempt to) satisfy the constraints.
  RankTwoTensor dstress; // change in stress
  std::vector<Real> dpm; // change in plasticity multipliers ("consistency parameters").  For ALL contraints (active and deactive)
  std::vector<Real>  dintnl; // change in internal parameters.  For ALL internal params (active and deactive)


  // The constraints that have been deactivated for this NR step
  // due to the flow directions being linearly dependent
  std::vector<bool> deact_ld;
  deact_ld.assign(_num_f, false);


  /* After NR and linesearch, if _deactivation_scheme == "optimized", the
   * active plasticity multipliers are checked for non-negativity.  If some
   * are negative then they are deactivated forever, and the NR step is
   * re-done starting from the *_before_step quantities recorded above
   */
  bool constraints_changing = true;

  while (constraints_changing)
  {
    constraints_changing = false;


    // calculate dstress, dpm and dintnl for one full Newton-Raphson step
    nrStep(stress, intnl_old, intnl, pm, E_inv, delta_dp, dstress, dpm, dintnl, active, deact_ld);


    // perform a line search
    // The line-search will exit with updated values
    successful_step = lineSearch(nr_res2, stress, intnl_old, intnl, pm, E_inv, delta_dp, dstress, dpm, dintnl, f, epp, ic, active, deact_ld);


    if (!successful_step)
      // completely bomb out
      break;



    // See if any active constraints need to be removed, and the step re-done
    if (deactivation_scheme == "optimized")
    {
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        if (active[alpha] && pm[alpha] < 0.0)
          constraints_changing = true;
      if (constraints_changing)
      {
        stress = stress_before_step;
        delta_dp = delta_dp_before_step;
        nr_res2 = nr_res2_before_step;
        for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        {
          if (active[alpha] && pm[alpha] < 0.0)
          {
            // turn off the constraint forever
            Moose::out << "Turning off constraint " << alpha << " forever\n";
            active[alpha] = false;
            pm_before_step[alpha] = 0.0;
            intnl_before_step[alpha] = intnl_old[alpha]; // don't want to muck-up hardening!
          }
          intnl[alpha] = intnl_before_step[alpha];
          pm[alpha] = pm_before_step[alpha];
        }
        if (numberActive(active) == 0)
        {
          // completely bomb out
          constraints_changing = false;
          successful_step = false;
          break;
        }
      }
    }




    // reinstate any active values that have been turned off due to linear-dependence
    bool reinstated_actives = reinstateLinearDependentConstraints(deact_ld);

    // if active constraints were reinstated then nr_res2, f, and ic need to be re-calculated
    if (reinstated_actives && !constraints_changing) // latter condition is for efficiency, since in that case we'll have to re-do this NR step anyway
    {

      bool completely_converged = true;
      if (successful_step && nr_res2 < 0.5)
      {
        // Here we have converged to the correct solution if
        // all the yield functions are < 0.
        // This is quite tricky - perhaps i can refactor to make it more obvious.
        // However, because actives are now reinstated, the residual2
        // calculation below will give nr_res2 > 0.5
        // Therefore, check for this case
        std::vector<bool> all_active;
        all_active.assign(_num_f, true);
        calculateConstraints(stress, intnl_old, intnl, pm, delta_dp, f, epp, ic, all_active, false, deact_ld);
        for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
          if (f[alpha] > _f[alpha]->_f_tol)
            completely_converged = false;
      }
      else
        completely_converged = false;

      if (!completely_converged)
      {
        calculateConstraints(stress, intnl_old, intnl, pm, delta_dp, f, epp, ic, active, false, deact_ld);
        nr_res2 = residual2(pm, f, epp, ic, active, deact_ld) + extra_trial(stress, intnl, active, deact_ld);
      }

      // if completely_converged then we'll exit since then also !constraints_changing
    }


  } // ends the "constraints_changing" while loop

  return successful_step;
}

bool
FiniteStrainMultiPlasticity::reinstateLinearDependentConstraints(std::vector<bool> & deactivated_due_to_ld)
{
  bool reinstated_actives = false;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (deactivated_due_to_ld[alpha])
      reinstated_actives = true;
  deactivated_due_to_ld.assign(_num_f, false);
  return reinstated_actives;
}

unsigned
FiniteStrainMultiPlasticity::numberActive(const std::vector<bool> & active)
{
  unsigned num_active = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      num_active++;
  return num_active;
}

bool
FiniteStrainMultiPlasticity::checkAndApplyKuhnTucker(const std::vector<Real> & f, const std::vector<Real> & pm, std::vector<bool> & active)
{
  bool kt = true;
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    if (active[alpha])
    {
      if (f[ind++] < -_f[alpha]->_f_tol)
        if (pm[alpha] != 0)
        {
          kt = false;
          active[alpha] = false;
        }
    }
    else if (pm[alpha] != 0)
      mooseError("Crash due to plastic multiplier not being zero.  This occurred because of poor coding!!");
  }
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (pm[alpha] < 0)
    {
      kt = false;
      active[alpha] = false;
    }
  return kt;
}


void
FiniteStrainMultiPlasticity::calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic, const std::vector<bool> & active, const bool & deactivate_if_linear_dependence, std::vector<bool> & deactivated_due_to_ld)
// NOTE: this must not change "deactivated_due_to_ld" if deactivate_if_linear_dependence == false
{

  // construct constraints
  //
  // epp = pm*r - E_inv*(trial_stress - stress) = pm*r - delta_dp
  // f = yield function    [only the "active_now" constraints]
  // ic = intnl - intnl_old + pm*h
  //
  // Here pm*r = sum_{active_alpha} pm[alpha]*r[alpha].  Note that this contains all the "active" constraints,
  //             even the ones that have been deactivated_due_to_ld.  r is a std::vector containing all the
  //             active flow directions.
  // Also yield_function contains only the "active_now" constraints.  f is a std::vector containing only
  //             these yield functions
  // Also pm*h = sum_{active_now_alpha} pm[alpha]*h[alpha].  Note that this only contains the "active_now"
  //             hardening potentials.  h is a std::vector containing only these "active_now" ones.

  unsigned num_active = numberActive(active);

  // flow direction
  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, active, num_active, r);


  if (deactivate_if_linear_dependence)
  {
    // yield functions
    yieldFunction(stress, intnl, active, num_active, f);
    // eliminate
    eliminateLinearDependence(stress, intnl, f, r, active, num_active, deactivated_due_to_ld);
  }

  unsigned num_deactivated_due_to_ld = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (deactivated_due_to_ld[alpha])
      num_deactivated_due_to_ld++;

  epp = RankTwoTensor();
  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      epp += pm[alpha]*r[ind++]; // note, even the deactivated_due_to_ld must get added in
  epp -= delta_dp;


  // the active constraints without the deactivated_due_to_ld
  std::vector<bool> active_now;
  unsigned num_active_now = 0;
  active_now.resize(_num_f, false);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha] && !deactivated_due_to_ld[alpha])
    {
      // do not need the deactivated_due_to_ld since those plastic muldipliers, and hence the intnl values aren't changing this NR step.
      active_now[alpha] = true;
      num_active_now++;
    }

  // yield functions
  if (!deactivate_if_linear_dependence)
    // if deactivate_if_linear_dependence then f has been calculated above
    yieldFunction(stress, intnl, active_now, num_active_now, f);

  // internal constraints
  std::vector<Real> h;
  hardPotential(stress, intnl, active_now, num_active_now, h);

  ic.resize(num_active_now);
  ind = 0;
  for (unsigned a = 0 ; a < _num_f ; ++a)
    if (active_now[a])
    {
      ic[ind] = intnl[a] - intnl_old[a] + pm[a]*h[ind];
      ind++;
    }
}

void
FiniteStrainMultiPlasticity::eliminateLinearDependence(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f, const std::vector<RankTwoTensor> & r, const std::vector<bool> & active, unsigned num_active, std::vector<bool> & deactivated_due_to_ld)
{
  deactivated_due_to_ld.resize(_num_f, false);

  if (num_active <= 1)
    return;

  std::vector<double> s(std::min(num_active, unsigned(6)));
  int info = singularValuesOfR(r, s);
  if (info != 0)
    mooseError("In finding the SVD in the return-map algorithm, the PETSC LAPACK gesvd routine returned with error code " << info);

  // num_lin_dep are the number of linearly dependent
  // "r vectors", if num_active <= 6
  unsigned int num_lin_dep = 0;
  for (unsigned i = s.size() - 1 ; i > 0 ; --i)
    if (s[i] < _svd_tol*s[0])
      num_lin_dep++;
    else
      break;

  if (num_lin_dep == 0 && num_active <= 6)
    return;


  // From here on, some flow directions are linearly dependent

  // Find the "distance" of the current (stress, internal) configuration
  // from the yield surfaces.  This distance will not be precise, but
  // i want to preferentially deactivate yield surfaces that are close
  // to the current stress point.
  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, active, num_active, df_dstress);

  typedef std::pair<Real, unsigned> pair_for_sorting;
  std::vector<pair_for_sorting> dist(num_active);
  for (unsigned i = 0 ; i < num_active ; ++i)
  {
    dist[i].first = f[i]/df_dstress[i].L2norm();
    dist[i].second = i;
  }
  std::sort(dist.begin(), dist.end()); // sorted in ascending order

  // There is a potential problem when we have equal f[i], for it can give oscillations
  bool equals_detected = false;
  for (unsigned i = 0 ; i < num_active - 1 ; ++i)
    if (std::abs(dist[i].first - dist[i + 1].first) < _min_f_tol)
    {
      equals_detected = true;
      dist[i].first += _min_f_tol*(MooseRandom::rand() - 0.5);
    }
  if (equals_detected)
    std::sort(dist.begin(), dist.end()); // sorted in ascending order


  std::vector<bool> scheduled_for_deactivation;
  scheduled_for_deactivation.assign(num_active, false);


  unsigned current_yf;
  std::vector<RankTwoTensor> r_tmp(1);
  current_yf = dist[num_active - 1].second;
  r_tmp[0] = r[current_yf];
  unsigned num_kept_active = 1;
  // Refactor the following to make smarter for num_active > 6
  for (unsigned yf_to_try = 2 ; yf_to_try <= num_active ; ++yf_to_try)
  {
    current_yf = dist[num_active - yf_to_try].second;
    if (num_active == 2) // shortcut to we don't have to singularValuesOfR
      scheduled_for_deactivation[current_yf] = true;
    else if (num_kept_active >= 6) // shortcut to we don't have to singularValuesOfR: there can never be > 6 linearly-independent r vectors
      scheduled_for_deactivation[current_yf] = true;
    else
    {
      r_tmp.push_back(r[current_yf]);
      info = singularValuesOfR(r_tmp, s);
      if (s[s.size() - 1] < _svd_tol*s[0])
      {
        scheduled_for_deactivation[current_yf] = true;
        r_tmp.pop_back();
        num_lin_dep--;
      }
      else
        num_kept_active++;
      if (num_lin_dep == 0 && num_active <= 6)
        // have taken out all the vectors that were linearly dependent
        // so no point continuing
        break;
    }
  }

  // record the original values of f in order to extract only the linear-dependent ones
  std::vector<Real> f_old(num_active);
  for (unsigned i = 0 ; i < num_active ; ++i)
  {
    f_old[i] = f[i];
  }

  // prepare f for only the active ones
  f.resize(num_active);

  // deactivate, and calculate the new f vector
  int old_active_number = -1;
  int new_active_number = -1;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
    {
      old_active_number++;
      if (scheduled_for_deactivation[old_active_number])
        deactivated_due_to_ld[alpha] = true;
      else
      {
        new_active_number++;
        f[new_active_number] = f_old[old_active_number];
      }
    }
}


int
FiniteStrainMultiPlasticity::singularValuesOfR(const std::vector<RankTwoTensor> & r, std::vector<Real> & s)
{
  int bm = r.size();
  int bn = 6;

  s.resize(std::min(bm, bn));

  // prepare for gesvd or gesdd routine provided by PETSc
  // Want to find the singular values of matrix
  //     (  r[0](0,0) r[0](0,1) r[0](0,2) r[0](1,1) r[0](1,2) r[0](2,2)  )
  //     (  r[1](0,0) r[1](0,1) r[1](0,2) r[1](1,1) r[1](1,2) r[1](2,2)  )
  // a = (  r[2](0,0) r[2](0,1) r[2](0,2) r[2](1,1) r[2](1,2) r[2](2,2)  )
  //     (  r[3](0,0) r[3](0,1) r[3](0,2) r[3](1,1) r[3](1,2) r[3](2,2)  )
  //     (  r[4](0,0) r[4](0,1) r[4](0,2) r[4](1,1) r[4](1,2) r[4](2,2)  )
  // bm = 5

  std::vector<double> a(bm*6);
  // Fill in the a "matrix" by going down columns
  unsigned ind = 0;
  for (int col = 0 ; col < 3 ; ++col)
    for (int row = 0 ; row < bm ; ++row)
      a[ind++] = r[row](0, col);
  for (int col = 3 ; col < 5 ; ++col)
    for (int row = 0 ; row < bm ; ++row)
      a[ind++] = r[row](1, col-2);
  for (int row = 0 ; row < bm ; ++row)
    a[ind++] = r[row](2, 2);


  // u and vt are dummy variables because they won't
  // get referenced due to the "N" and "N" choices
  int sizeu = 1;
  std::vector<double> u(sizeu);
  int sizevt = 1;
  std::vector<double> vt(sizevt);

  int sizework = 16*(bm + 6); // this is above the lowerbound specified in the LAPACK doco
  std::vector<double> work(sizework);

  int info;

  LAPACKgesvd_("N", "N", &bm, &bn , &a[0], &bm, &s[0], &u[0], &sizeu, &vt[0], &sizevt, &work[0], &sizework, &info);

  return info;
}

void
FiniteStrainMultiPlasticity::calculateRHS(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & rhs, const std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld)
{
  std::vector<Real> f; // the yield functions
  RankTwoTensor epp; // the plastic-strain constraint ("direction constraint")
  std::vector<Real> ic; // the "internal constraints"

  calculateConstraints(stress, intnl_old, intnl, pm, delta_dp, f, epp, ic, active, true, deactivated_due_to_ld);

  unsigned num_active_f = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha] && !deactivated_due_to_ld[alpha])
      num_active_f++;

  unsigned int dim = 3;
  unsigned int system_size = 6 + num_active_f + num_active_f; // "6" comes from symmeterizing epp, num_active_f comes from "f", num_active_f comes from "ic"

  rhs.resize(system_size);

  // rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[_num_active_f], ic[0], ic[1], ..., ic[num_active_f])
  // notice the appearance of only the i>=j components

  unsigned ind = 0;
  for (unsigned i = 0 ; i < dim ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
      rhs[ind++] = -epp(i, j);
  for (unsigned i = 0 ; i < num_active_f ; ++i)
    rhs[ind++] = -f[i];
  for (unsigned i = 0 ; i < num_active_f ; ++i)
    rhs[ind++] = -ic[i];
}


Real
FiniteStrainMultiPlasticity::extra_trial(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld)
{
  Real nr_res2 = 0;

  std::vector<Real> f;
  unsigned num_act = numberActive(active);
  yieldFunction(stress, intnl, active, num_act, f);

  unsigned ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
    {
      if (deactivated_due_to_ld[alpha] && f[ind] > 0)
        nr_res2 += 0.5*std::pow(f[ind]/_f[alpha]->_f_tol, 2);
      ind++;
    }
  Moose::out << "extra bit is " << nr_res2 << "\n";
  return nr_res2;
}

Real
FiniteStrainMultiPlasticity::residual2(const std::vector<Real> & pm, const std::vector<Real> & f, const RankTwoTensor & epp, const std::vector<Real> & ic, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld)
{
  Real nr_res2 = 0;
  unsigned ind = 0;

  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha] && !deactivated_due_to_ld[alpha])
    {
      if (!(pm[alpha] == 0 && f[ind] <= 0) )
        nr_res2 += 0.5*std::pow( f[ind]/_f[alpha]->_f_tol, 2);
      ind++;
    }

  nr_res2 += 0.5*std::pow(epp.L2norm()/_epp_tol, 2);

  ind = 0;
  for (unsigned a = 0 ; a < _num_f; ++a)
    if (active[a] && !deactivated_due_to_ld[a])
      nr_res2 += 0.5*std::pow(ic[ind++]/_f[a]->_ic_tol, 2);


  return nr_res2;
}


void
FiniteStrainMultiPlasticity::calculateJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld, std::vector<std::vector<Real> > & jac)
{
  unsigned num_active = numberActive(active);

  unsigned num_active_now = 0;
  std::vector<bool> active_now;
  active_now.assign(_num_f, false);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha] && !deactivated_due_to_ld[alpha])
    {
      active_now[alpha] = true;
      num_active_now++;
    }


  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, active_now, num_active_now, df_dstress);

  std::vector<Real> df_dintnl;
  dyieldFunction_dintnl(stress, intnl, active_now, num_active_now, df_dintnl);

  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, active, num_active, r);

  std::vector<RankFourTensor> dr_dstress;
  dflowPotential_dstress(stress, intnl, active, num_active, dr_dstress);

  std::vector<RankTwoTensor> dr_dintnl;
  dflowPotential_dintnl(stress, intnl, active, num_active, dr_dintnl);

  std::vector<Real> h;
  hardPotential(stress, intnl, active_now, num_active_now, h);

  std::vector<RankTwoTensor> dh_dstress;
  dhardPotential_dstress(stress, intnl, active_now, num_active_now, dh_dstress);

  std::vector<Real> dh_dintnl;
  dhardPotential_dintnl(stress, intnl, active_now, num_active_now, dh_dintnl);


  // construct matrix entries
  // In the following
  // epp = pm*r - E_inv*(trial_stress - stress) = pm*r - delta_dp
  // f = yield function    [only the "active_now" constraints]
  // ic = intnl - intnl_old + pm*h
  //
  // Here pm*r = sum_{active_alpha} pm[alpha]*r[alpha].  Note that this contains all the "active" constraints,
  //             even the ones that have been deactivated_due_to_ld.  r is a std::vector containing all the
  //             active flow directions.
  // Also yield_function contains only the "active_now" constraints.  f is a std::vector containing only
  //             these yield functions
  // Also pm*h = sum_{active_now_alpha} pm[alpha]*h[alpha].  Note that this only contains the "active_now"
  //             hardening potentials.  h is a std::vector containing only these "active_now" ones.
  //
  // The degrees of freedom are:
  //   the 6 components of stress
  //   the active_now plastic multipliers, pm
  //   the active_now internal parameters, intnl
  // Hence we only find derivatives wrt these parameters

  unsigned ind = 0;
  unsigned active_now_ind = 0;

  // d(epp)/dstress = sum_{active alpha} pm[alpha]*dr_dstress
  RankFourTensor depp_dstress;
  ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha])
      depp_dstress += pm[alpha]*dr_dstress[ind++];
  depp_dstress += E_inv;

  // d(epp)/dpm_{active_now_index} = r_{active_now_index}
  std::vector<RankTwoTensor> depp_dpm;
  depp_dpm.resize(num_active_now);
  ind = 0;
  active_now_ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    if (active_now[alpha])
      depp_dpm[active_now_ind++] = r[ind];
    if (active[alpha])
      ind++;
  }

  // d(epp)/dintnl_{active_now_index} = pm[active_now_index]*dr_dintnl[active_now_index]
  std::vector<RankTwoTensor> depp_dintnl;
  depp_dintnl.assign(num_active_now, RankTwoTensor());
  active_now_ind = 0;
  ind = 0;
  for (unsigned alpha = 0; alpha < _num_f ; ++alpha)
  {
    if (active_now[alpha])
      depp_dintnl[active_now_ind++] = pm[alpha]*dr_dintnl[ind];
    if (active[alpha])
      ind++;
  }


  // df_dstress has been calculated above
  // df_dpm is always zero
  // df_dintnl has been calculated above

  std::vector<RankTwoTensor> dic_dstress;
  dic_dstress.assign(num_active_now, RankTwoTensor());
  ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active_now[alpha])
    {
      dic_dstress[ind] += pm[alpha]*dh_dstress[ind];
      ind++;
    }


  std::vector<std::vector<Real> > dic_dpm;
  dic_dpm.resize(num_active_now);
  for (unsigned i = 0 ; i < num_active_now ; ++i)
    dic_dpm[i].assign(num_active_now, 0);
  ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active_now[alpha])
    {
      dic_dpm[ind][ind] = h[ind];
      ind++;
    }

  std::vector<std::vector<Real> > dic_dintnl;
  dic_dintnl.resize(num_active_now);
  for (unsigned i = 0 ; i < num_active_now ; ++i)
    dic_dintnl[i].assign(num_active_now, 0);
  ind = 0;
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active_now[alpha])
    {
      dic_dintnl[ind][ind] = 1 + pm[alpha]*dh_dintnl[ind];
      ind++;
    }



  /**
   * now construct the Jacobian
   * It is:
   * ( depp_dstress   depp_dpm  depp_dintnl )
   * (  df_dstress       0      df_dintnl   )
   * ( dic_dstress    dic_dpm   dic_dintnl  )
   * For the "epp" terms, only the i>=j components are kept in the RHS, so only these terms are kept here too
   */

  unsigned int dim = 3;
  unsigned int system_size = 6 + num_active_now + num_active_now; // "6" comes from symmeterizing epp
  jac.resize(system_size);
  for (unsigned i = 0 ; i < system_size ; ++i)
    jac[i].assign(system_size, 0);

  unsigned int row_num = 0;
  unsigned int col_num = 0;
  for (unsigned i = 0 ; i < dim ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
    {
      for (unsigned k = 0 ; k < dim ; ++k)
        for (unsigned l = 0 ; l <= k ; ++l)
          jac[col_num][row_num++] = depp_dstress(i, j, k, l) + (k != l ? depp_dstress(i, j, l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
      for (unsigned alpha = 0 ; alpha < num_active_now ; ++alpha)
        jac[col_num][row_num++] = depp_dpm[alpha](i, j);
      for (unsigned a = 0 ; a < num_active_now ; ++a)
        jac[col_num][row_num++] = depp_dintnl[a](i, j);
      row_num = 0;
      col_num++;
    }

  for (unsigned alpha = 0 ; alpha < num_active_now ; ++alpha)
  {
    for (unsigned k = 0 ; k < dim ; ++k)
      for (unsigned l = 0 ; l <= k ; ++l)
        jac[col_num][row_num++] = df_dstress[alpha](k, l) + (k != l ? df_dstress[alpha](l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
    for (unsigned beta = 0 ; beta < num_active_now ; ++beta)
      jac[col_num][row_num++] = 0;
    for (unsigned a = 0 ; a < num_active_now ; ++a)
      if (a == alpha)
        jac[col_num][row_num++] = df_dintnl[alpha];
      else
        jac[col_num][row_num++] = 0;
    row_num = 0;
    col_num++;
  }

  for (unsigned a = 0 ; a < num_active_now ; ++a)
  {
    for (unsigned k = 0 ; k < dim ; ++k)
      for (unsigned l = 0 ; l <= k ; ++l)
        jac[col_num][row_num++] = dic_dstress[a](k, l) + (k != l ? dic_dstress[a](l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
    for (unsigned alpha = 0 ; alpha < num_active_now ; ++alpha)
      jac[col_num][row_num++] = dic_dpm[a][alpha];
    for (unsigned b = 0 ; b < num_active_now ; ++b)
      jac[col_num][row_num++] = dic_dintnl[a][b];
    row_num = 0;
    col_num++;
  }

}

void
FiniteStrainMultiPlasticity::nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl, const std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld)
{
  // Calculate RHS and Jacobian
  std::vector<Real> rhs;
  calculateRHS(stress, intnl_old, intnl, pm, delta_dp, rhs, active, deactivated_due_to_ld);

  std::vector<std::vector<Real> > jac;
  calculateJacobian(stress, intnl, pm, E_inv, active, deactivated_due_to_ld, jac);



  // prepare for LAPACKgesv_ routine provided by PETSc
  int system_size = rhs.size();

  std::vector<double> a(system_size*system_size);
  // Fill in the a "matrix" by going down columns
  unsigned ind = 0;
  for (int col = 0 ; col < system_size ; ++col)
    for (int row = 0 ; row < system_size ; ++row)
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
  for (unsigned i = 0 ; i < dim ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
      dstress(i, j) = dstress(j, i) = rhs[ind++];
  dpm.assign(_num_f, 0);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    if (active[alpha] && !deactivated_due_to_ld[alpha])
      dpm[alpha] = rhs[ind++];
  dintnl.assign(_num_f, 0);
  for (unsigned a = 0 ; a < _num_f ; ++a)
    if (active[a] && !deactivated_due_to_ld[a])
      dintnl[a] = rhs[ind++];
}


bool
FiniteStrainMultiPlasticity::lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic, std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld)
{
  // Line search algorithm straight out of "Numerical Recipes"

  bool success = true; // return value: will be false if linesearch couldn't reduce the residual-squared

  // Aim is to decrease residual2

  Real lam = 1.0; // the line-search parameter: 1.0 is a full Newton step
  Real lam_min = 1E-10; // minimum value of lam allowed - perhaps this should be dynamically calculated?
  bool line_searching = true;
  Real f0 = nr_res2; // initial value of residual2
  Real slope = -2*nr_res2; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but i hope the nrStep would warn if there were problems.
  Real tmp_lam; // cached value of lam used in quadratic & cubic line search
  Real f2; // cached value of f = residual2 used in the cubic in the line search
  Real lam2; // cached value of lam used in the cubic in the line search


  // pm during the line-search
  std::vector<Real> ls_pm;
  ls_pm.resize(_num_f);

  // delta_dp during the line-search
  RankTwoTensor ls_delta_dp;

  // internal parameter during the line-search
  std::vector<Real> ls_intnl;
  ls_intnl.resize(_num_f);

  // stress during the line-search
  RankTwoTensor ls_stress;



  while (line_searching)
  {
    // update the variables using this line-search parameter
    for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
      ls_pm[alpha] = pm[alpha] + dpm[alpha]*lam;
    ls_delta_dp = delta_dp - E_inv*dstress*lam;
    for (unsigned a = 0 ; a < _num_f ; ++ a)
      ls_intnl[a] = intnl[a] + dintnl[a]*lam;
    ls_stress = stress + dstress*lam;

    // calculate the new active yield functions, epp and active internal constraints
    calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, epp, ic, active, false, deactivated_due_to_ld);

    // calculate the new residual-squared
    nr_res2 = residual2(ls_pm, f, epp, ic, active, deactivated_due_to_ld) + extra_trial(ls_stress, ls_intnl, active, deactivated_due_to_ld);


    if (nr_res2 < f0 + 1E-4*lam*slope)
    {
      line_searching = false;
      break;
    }
    else if (lam < lam_min)
    {
      success = false;
      // restore plastic multipliers, yield functions, etc to original values
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        ls_pm[alpha] = pm[alpha];
      ls_delta_dp = delta_dp;
      for (unsigned a = 0 ; a < _num_f ; ++ a)
        ls_intnl[a] = intnl[a];
      ls_stress = stress;
      calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, epp, ic, active, false, deactivated_due_to_ld);
      nr_res2 = residual2(ls_pm, f, epp, ic, active, deactivated_due_to_ld) + extra_trial(ls_stress, ls_intnl, active, deactivated_due_to_ld);
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
  }

  // assign the quantities found in the line-search
  // back to the originals
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    pm[alpha] = ls_pm[alpha];
  delta_dp = ls_delta_dp;
  for (unsigned a = 0 ; a < _num_f ; ++ a)
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
FiniteStrainMultiPlasticity::checkDerivatives()
{
  _console << "\n ++++++++++++++ \nChecking the derivatives\n";
  outputAndCheckDebugParameters();

  std::vector<bool> act;
  act.assign(_num_f, true);

  _console << "dyieldFunction_dstress.  Relative L2 norms.\n";
  std::vector<RankTwoTensor> df_dstress;
  std::vector<RankTwoTensor> fddf_dstress;
  dyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, act, _num_f, df_dstress);
  fddyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddf_dstress);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
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
  dflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, act, _num_f, dr_dstress);
  fddflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddr_dstress);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    _console << "alpha = " << alpha << " Relative L2norm = " << 2*(dr_dstress[alpha] - fddr_dstress[alpha]).L2norm()/(dr_dstress[alpha] + fddr_dstress[alpha]).L2norm() << "\n";
    _console << "Coded:\n";
    dr_dstress[alpha].print();
    _console << "Finite difference:\n";
    fddr_dstress[alpha].print();
  }

  _console << "dflowPotential_dintnl.  Relative L2 norms.\n";
  std::vector<RankTwoTensor> dr_dintnl;
  std::vector<RankTwoTensor> fddr_dintnl;
  dflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, act, _num_f, dr_dintnl);
  fddflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, fddr_dintnl);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    _console << "alpha = " << alpha << " Relative L2norm = " << 2*(dr_dintnl[alpha] - fddr_dintnl[alpha]).L2norm()/(dr_dintnl[alpha] + fddr_dintnl[alpha]).L2norm() << "\n";
    _console << "Coded:\n";
    dr_dintnl[alpha].print();
    _console << "Finite difference:\n";
    fddr_dintnl[alpha].print();
  }

}

void
FiniteStrainMultiPlasticity::fddyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(_num_f, RankTwoTensor());

  std::vector<bool> act;
  act.assign(_num_f, true);

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<Real> fep, fep_minus;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      stressep = stress;
      // do a central difference to attempt to capture discontinuities
      // such as those encountered in tensile and Mohr-Coulomb
      stressep(i, j) += ep/2.0;
      yieldFunction(stressep, intnl, act, _num_f, fep);
      stressep(i, j) -= ep;
      yieldFunction(stressep, intnl, act, _num_f, fep_minus);
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        df_dstress[alpha](i, j) = (fep[alpha] - fep_minus[alpha])/ep;
    }
}

void
FiniteStrainMultiPlasticity::fddflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.assign(_num_f, RankFourTensor());

  std::vector<bool> act;
  act.assign(_num_f, true);

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<RankTwoTensor> rep, rep_minus;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      stressep = stress;
      stressep(i, j) += ep/2.0;
      flowPotential(stressep, intnl, act, _num_f, rep);
      stressep(i, j) -= ep;
      flowPotential(stressep, intnl, act, _num_f, rep_minus);
      for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
        for (unsigned k = 0 ; k < 3 ; ++k)
          for (unsigned l = 0 ; l < 3 ; ++l)
            dr_dstress[alpha](k, l, i, j) = (rep[alpha](k, l) - rep_minus[alpha](k, l))/ep;
    }
}

void
FiniteStrainMultiPlasticity::fddflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & dr_dintnl)
{
  dr_dintnl.resize(_num_f);

  std::vector<bool> act;
  act.assign(_num_f, true);

  std::vector<RankTwoTensor> origr;
  flowPotential(stress, intnl, act, _num_f, origr);

  std::vector<Real> intnlep;
  intnlep.resize(_num_f);
  for (unsigned a = 0 ; a < _num_f ; ++a)
    intnlep[a] = intnl[a];
  Real ep;
  std::vector<RankTwoTensor> rep;
  for (unsigned a = 0 ; a < _num_f ; ++a)
  {
    ep = _fspb_debug_intnl_change[a];
    intnlep[a] += ep;
    flowPotential(stress, intnlep, act, _num_f, rep);
    dr_dintnl[a] = (rep[a] - origr[a])/ep;
    intnlep[a] -= ep;
  }
}


void
FiniteStrainMultiPlasticity::checkJacobian()
{
  _console << "\n ++++++++++++++ \nChecking the Jacobian\n";
  outputAndCheckDebugParameters();

  std::vector<bool> act;
  act.assign(_num_f, true);
  std::vector<bool> deactivated_due_to_ld;
  deactivated_due_to_ld.assign(_num_f, false);

  RankFourTensor E_inv = _elasticity_tensor[_qp].invSymm();
  RankTwoTensor delta_dp = -E_inv*_fspb_debug_stress;

  std::vector<std::vector<Real> > jac;
  calculateJacobian(_fspb_debug_stress, _fspb_debug_intnl, _fspb_debug_pm, E_inv, act, deactivated_due_to_ld, jac);

  std::vector<std::vector<Real> > fdjac;
  fdJacobian(_fspb_debug_stress, _intnl_old[_qp], _fspb_debug_intnl, _fspb_debug_pm, delta_dp, E_inv, fdjac);

  _console << "Hand-coded Jacobian:\n";
  for (unsigned row = 0 ; row < jac.size() ; ++row)
  {
    for (unsigned col = 0 ; col < jac.size() ; ++col)
      _console << jac[row][col] << " ";
    _console << "\n";
  }

  _console << "Finite difference Jacobian:\n";
  for (unsigned row = 0 ; row < fdjac.size() ; ++row)
  {
    for (unsigned col = 0 ; col < fdjac.size() ; ++col)
      _console << fdjac[row][col] << " ";
    _console << "\n";
  }
}

void
FiniteStrainMultiPlasticity::fdJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac)
{
  std::vector<bool> active;
  active.assign(_num_f, true);

  std::vector<bool> deactivated_due_to_ld;

  std::vector<Real> orig_rhs;
  calculateRHS(stress, intnl_old, intnl, pm, delta_dp, orig_rhs, active, deactivated_due_to_ld);

  unsigned int system_size = orig_rhs.size();
  jac.resize(system_size);
  for (unsigned row = 0 ; row < system_size ; ++row)
    jac[row].assign(system_size, 0);


  std::vector<Real> rhs_ep;
  unsigned col = 0;

  RankTwoTensor stressep;
  RankTwoTensor delta_dpep;
  Real ep = _fspb_debug_stress_change;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
    {
      stressep = stress;
      stressep(i, j) += ep;
      if (i != j)
        stressep(j, i) += ep;
      delta_dpep = delta_dp;
      for (unsigned k = 0; k < 3 ; ++k)
        for (unsigned l = 0 ; l < 3 ; ++l)
        {
          delta_dpep(k, l) -= E_inv(k, l, i, j)*ep;
          if (i != j)
            delta_dpep(k, l) -= E_inv(k, l, j, i)*ep;
        }
      active.assign(_num_f, true);
      calculateRHS(stressep, intnl_old, intnl, pm, delta_dpep, rhs_ep, active, deactivated_due_to_ld);
      for (unsigned row = 0 ; row < system_size ; ++row)
        jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
      col++;
    }

  std::vector<Real> pmep;
  pmep.resize(_num_f);
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    pmep[alpha] = pm[alpha];
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
  {
    ep = _fspb_debug_pm_change[alpha];
    pmep[alpha] += ep;
    active.assign(_num_f, true);
    calculateRHS(stress, intnl_old, intnl, pmep, delta_dp, rhs_ep, active, deactivated_due_to_ld);
    for (unsigned row = 0 ; row < system_size ; ++row)
      jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
    pmep[alpha] -= ep;
    col++;
  }

  std::vector<Real> intnlep;
  intnlep.resize(_num_f);
  for (unsigned a = 0 ; a < _num_f ; ++a)
    intnlep[a] = intnl[a];
  for (unsigned a = 0 ; a < _num_f ; ++a)
  {
    ep = _fspb_debug_intnl_change[a];
    intnlep[a] += ep;
    active.assign(_num_f, true);
    calculateRHS(stress, intnl_old, intnlep, pm, delta_dp, rhs_ep, active, deactivated_due_to_ld);
    for (unsigned row = 0 ; row < system_size ; ++row)
      jac[row][col] = -(rhs_ep[row] - orig_rhs[row])/ep; // remember jacobian = -d(rhs)/d(something)
    intnlep[a] -= ep;
    col++;
  }
}


void
FiniteStrainMultiPlasticity::outputAndCheckDebugParameters()
{
  _console << "stress = \n";
  _fspb_debug_stress.print();

  if (_fspb_debug_pm.size() != _num_f || _fspb_debug_intnl.size() != _num_f || _fspb_debug_pm_change.size() != _num_f || _fspb_debug_intnl_change.size() != _num_f)
    mooseError("The debug parameters have the wrong size\n");

  _console << "plastic multipliers =\n";
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    _console << _fspb_debug_pm[alpha] << "\n";

  _console << "internal parameters =\n";
  for (unsigned a = 0 ; a < _num_f ; ++a)
    _console << _fspb_debug_intnl[a] << "\n";

  _console << "finite-differencing parameter for stress-changes:\n" << _fspb_debug_stress_change  << "\n";
  _console << "finite-differencing parameter(s) for plastic-multiplier(s):\n";
  for (unsigned alpha = 0 ; alpha < _num_f ; ++alpha)
    _console << _fspb_debug_pm_change[alpha] << "\n";
  _console << "finite-differencing parameter(s) for internal-parameter(s):\n";
  for (unsigned a = 0 ; a < _num_f ; ++a)
    _console << _fspb_debug_intnl_change[a] << "\n";
}
