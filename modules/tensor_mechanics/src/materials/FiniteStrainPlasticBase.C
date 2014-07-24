#include "FiniteStrainPlasticBase.h"
#include "ReturnMapUtils.h" // for Newton-Raphson routines

template<>
InputParameters validParams<FiniteStrainPlasticBase>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRangeCheckedParam<unsigned int>("max_NR_iterations", 20, "max_NR_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addRequiredParam<std::vector<Real> >("yield_function_tolerance", "If the yield function is less than this amount, the (stress, internal parameters) are deemed admissible.  A vector of tolerances must be entered for the multi-surface case");
  params.addParam<std::vector<Real> >("internal_constraint_tolerance", "The Newton-Raphson process is only deemed converged if the internal constraint is less than this.  A vector of tolerances must be entered for the case with more than one internal parameter");
  params.addRequiredRangeCheckedParam<Real>("direction_tolerance", "direction_tolerance>0", "The Newton-Raphson process is only deemed converged if the direction constraints have L2 norm less than this.");
  params.addParam<int>("fspb_debug", 0, "Debug parameter for use by developers");
  params.addClassDescription("Base class for non-associative finite-strain plasticity");

  return params;
}

FiniteStrainPlasticBase::FiniteStrainPlasticBase(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
    _max_iter(getParam<unsigned int>("max_NR_iterations")),
    _f_tol(getParam<std::vector<Real> >("yield_function_tolerance")),
    _ic_tol(parameters.isParamValid("internal_constraint_tolerance") ? getParam<std::vector<Real> >("internal_constraint_tolerance") : std::vector<Real>(0)),
    _dirn_tol(getParam<Real>("direction_tolerance")),
    _fspb_debug(getParam<int>("fspb_debug")),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _intnl(declareProperty<std::vector<Real> >("plastic_internal_parameter")),
    _intnl_old(declarePropertyOld<std::vector<Real> >("plastic_internal_parameter")),
    _f(declareProperty<std::vector<Real> >("plastic_yield_function"))
{
  if (_f_tol.size() != numberOfYieldFunctions())
    mooseError("The number of yield_function_tolerance parameters must match the number of yield functions");
  for (unsigned alpha = 0 ; alpha < _f_tol.size() ; ++alpha)
    if (_f_tol[alpha] <= 0)
      mooseError("The yield_function_tolerance must be positive");

  if (_ic_tol.size() != numberOfInternalParameters())
    mooseError("The number of internal_constraint_tolerance parameters must match the number of internal parameters");
  for (unsigned a = 0 ; a < _ic_tol.size() ; ++a)
    if (_ic_tol[a] <= 0)
      mooseError("The internal_constraint_tolerance must be positive");
}


void
FiniteStrainPlasticBase::initQpStatefulProperties()
{
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
  _intnl[_qp].assign(numberOfInternalParameters(), 0);
  _intnl_old[_qp].assign(numberOfInternalParameters(), 0);
  _f[_qp].assign(numberOfYieldFunctions(), 0);
}

void
FiniteStrainPlasticBase::computeQpStress()
{
  // This is the total strain.  Pritam is re-working this part of the code (15 July 2014)
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

  preReturnMap();

  returnMap(_stress_old[_qp], _plastic_strain_old[_qp], _intnl_old[_qp], _strain_increment[_qp], _elasticity_tensor[_qp], _stress[_qp], _plastic_strain[_qp], _intnl[_qp], _f[_qp]);

  postReturnMap();

  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();

  //Rotate to plastic rate of deformation tensor the current configuration
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

void
FiniteStrainPlasticBase::returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f)
{

  // Assume this strain increment does not induce any plasticity
  // This is the elastic-predictor
  stress = stress_old + E_ijkl * delta_d; // the trial stress
  plastic_strain = plastic_strain_old;
  for (unsigned i = 0; i < intnl_old.size() ; ++i)
    intnl[i] = intnl_old[i];

  yieldFunction(stress, intnl, f);

  Real nr_res2 = 0;
  for (unsigned i = 0 ; i < f.size() ; ++i)
    nr_res2 += 0.5*std::pow( std::max(f[i], 0.0)/_f_tol[i], 2);

  if (nr_res2 < 0.5)
    // a purely elastic increment.
    // All output variables have been calculated
    return;


  // So, from here on we know that the trial stress
  // is inadmissible, and we have to return from that
  // value to the yield surface.  There are three
  // types of constraints we have to satisfy, listed
  // below, and calculated in calculateConstraints(...)

  // Direction constraint, must be zero (up to a tolerance)
  RankTwoTensor dirn;

  // Yield function constraint.
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
  // delta_dp = E^{-1}*(trial_stress - sig), where trial_stress = E*(strain - plastic_strain_old)
  RankTwoTensor delta_dp;

  // The "consistency parameters" (plastic multipliers)
  // Change in plastic strain in this timestep = ga*flowPotential
  // Each ga must be non-negative
  std::vector<Real> ga;
  ga.assign(numberOfYieldFunctions(), 0.0);



  // The Newton-Raphson loops
  unsigned int iter = 0;
  while (nr_res2 > 0.5 && iter < _max_iter)
  {
    iter++;

    // calculate dstress, dpm and dintnl for one full Newton-Raphson step
    nrStep(stress, intnl_old, intnl, ga, E_inv, delta_dp, f, dstress, dpm, dintnl);

    // perform a line search
    // The line-search will exit with updated values
    lineSearch(nr_res2, stress, intnl_old, intnl, ga, E_inv, delta_dp, dstress, dpm, dintnl, f, dirn, ic);
  }
  

  if (iter >= _max_iter)
  {
    stress = stress_old;
    for (unsigned i = 0; i < intnl_old.size() ; ++i)
      intnl[i] = intnl_old[i];
    _console << "Too many iterations in plasticity.\nYield function(s):\n";
    for (unsigned i = 0; i < f.size() ; ++i)
      _console << f[i] << "\n";
    _console << "Stress:\n";
    stress.print();
  }
  else
    plastic_strain += delta_dp;

}

void
FiniteStrainPlasticBase::calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & ga, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & dirn, std::vector<Real> & ic)
{
  // yield functions
  yieldFunction(stress, intnl, f);

  // direction constraints
  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, r);

  for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
    dirn += ga[alpha]*r[alpha];
  dirn -= delta_dp;

  // internal constraints
  std::vector<std::vector<Real> > h;
  hardPotential(stress, intnl, h);

  ic.resize(numberOfInternalParameters());
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++a)
  {
    ic[a] = intnl[a] - intnl_old[a];
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      ic[a] += ga[alpha]*h[a][alpha];
  }
}


Real
FiniteStrainPlasticBase::residual2(const std::vector<Real> & f, const RankTwoTensor & dirn, const std::vector<Real> & ic)
{
  Real nr_res2 = 0;
  for (unsigned alpha = 0 ; alpha < f.size() ; ++alpha)
    nr_res2 += 0.5*std::pow( std::max(f[alpha], 0.0)/_f_tol[alpha], 2);
  nr_res2 += 0.5*std::pow(dirn.L2norm()/_dirn_tol, 2);
  for (unsigned a = 0 ; a < ic.size() ; ++a)
    nr_res2 += 0.5*std::pow(ic[a]/_ic_tol[a], 2);
  return nr_res2;
}


void
FiniteStrainPlasticBase::nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & ga, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, const std::vector<Real> & f, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl)
{
  // construct quantities used in the Newton-Raphson linear system
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


  // construct RHS entries (the "f" are already constructed)
  RankTwoTensor dirn; // the "direction constraint"
  for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
    dirn += ga[alpha]*r[alpha];
  dirn -= delta_dp;

  std::vector<Real> ic; // the "internal constraints"
  ic.resize(numberOfInternalParameters());
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++a)
  {
    ic[a] = intnl[a] - intnl_old[a];
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      ic[a] += ga[alpha]*h[a][alpha];
  }

  // construct matrix entries
  RankFourTensor ddirn_dstress;
  for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
    ddirn_dstress += ga[alpha]*dr_dstress[alpha];
  ddirn_dstress += E_inv;

  std::vector<RankTwoTensor> ddirn_dpm;
  ddirn_dpm.resize(numberOfYieldFunctions());
  for (unsigned alpha = 0; alpha < numberOfYieldFunctions() ; ++alpha)
    ddirn_dpm[alpha] = r[alpha];

  std::vector<RankTwoTensor> ddirn_dintnl;
  ddirn_dintnl.assign(numberOfInternalParameters(), RankTwoTensor());
  for (unsigned a = 0; a < numberOfInternalParameters() ; ++a)
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      ddirn_dintnl[a] += ga[alpha]*dr_dintnl[alpha][a];

  std::vector<RankTwoTensor> dic_dstress;
  dic_dstress.assign(numberOfInternalParameters(), RankTwoTensor());
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++a)
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      dic_dstress[a] += ga[alpha]*dh_dstress[a][alpha];

  std::vector<std::vector<Real> > dic_dpm;
  dic_dpm.resize(numberOfInternalParameters());
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++a)
  {
    dic_dpm[a].resize(numberOfYieldFunctions());
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      dic_dpm[a][alpha] = h[a][alpha];
  }
  
  std::vector<std::vector<Real> > dic_dintnl;
  dic_dintnl.resize(numberOfInternalParameters());
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++a)
  {
    dic_dintnl[a].assign(numberOfInternalParameters(), 0);
    for (unsigned b = 0 ; b < numberOfInternalParameters() ; ++b)
      for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
	dic_dintnl[a][b] += ga[alpha]*dh_dintnl[a][alpha][b];
    dic_dintnl[a][a] += 1;
  }

  ReturnMapUtils::linearSolve(dirn, f, ic, ddirn_dstress, ddirn_dpm, ddirn_dintnl, df_dstress, df_dintnl, dic_dstress, dic_dpm, dic_dintnl, dstress, dpm, dintnl);

  if (_fspb_debug > 1)
    _console << "Solution error = " << ReturnMapUtils::solutionError(dirn, f, ic, ddirn_dstress, ddirn_dpm, ddirn_dintnl, df_dstress, df_dintnl, dic_dstress, dic_dpm, dic_dintnl, dstress, dpm, dintnl) << "\n";
}


void
FiniteStrainPlasticBase::lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & ga, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & dirn, std::vector<Real> & ic)
{
  // Line search algorithm straight out of "Numerical Recipes"

  // Aim is to decrease residual2

  Real lam = 1.0; // the line-search parameter: 1.0 is a full Newton step
  Real lam_min = 1E-7; // minimum value of lam allowed - perhaps this should be dynamically calculated?
  bool line_searching = true;
  Real f0 = nr_res2; // initial value of residual2
  Real slope = -nr_res2; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but i hope the nrStep would warn if there were problems.
  if (slope > 0)
    mooseError("Roundoff problem in weak-plane line search");
  Real tmp_lam; // cached value of lam used in quadratic & cubic line search
  Real f2; // cached value of f = residual2 used in the cubic in the line search
  Real lam2; // cached value of lam used in the cubic in the line search

  // ga during the line-search
  std::vector<Real> ls_ga; 
  ls_ga.resize(numberOfYieldFunctions());

  // delta_dp during the line-search
  RankTwoTensor ls_delta_dp;

  // internal parameter during the line-search
  std::vector<Real> ls_intnl;
  ls_intnl.resize(numberOfInternalParameters());
  
  // stress during the line-search
  RankTwoTensor ls_stress;

  while (line_searching)
  {
    // update the variables using this line-search parameter
    for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
      ls_ga[alpha] = ga[alpha] + dpm[alpha]*lam;
    ls_delta_dp = delta_dp - E_inv*dstress*lam;
    for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++ a)
      ls_intnl[a] = intnl[a] + dintnl[a]*lam;
    ls_stress = stress + dstress*lam;

    // calculate the new yield function, dirn and internal constraints
    calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_ga, ls_delta_dp, f, dirn, ic);

    // calculate the new residual-squared
    nr_res2 = residual2(f, dirn, ic);

    if (nr_res2 < f0 + 1E-4*lam*slope)
    {
      line_searching = false;
      break;
    }
    else if (lam < lam_min)
    {
      lam = 0.1;
      for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
	ls_ga[alpha] = ga[alpha] + dpm[alpha];
      ls_delta_dp = delta_dp - E_inv*dstress;
      for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++ a)
	ls_intnl[a] = intnl[a] + dintnl[a];
      ls_stress = stress + dstress;
      calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_ga, ls_delta_dp, f, dirn, ic);
      nr_res2 = residual2(f, dirn, ic);
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

  // assign the quantities found in the line-search
  // back to the originals
  for (unsigned alpha = 0 ; alpha < numberOfYieldFunctions() ; ++alpha)
    ga[alpha] = ls_ga[alpha];
  delta_dp = ls_delta_dp;
  for (unsigned a = 0 ; a < numberOfInternalParameters() ; ++ a)
    intnl[a] = ls_intnl[a];
  stress = ls_stress;
}
