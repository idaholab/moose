/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "EigenExecutionerBase.h"
#include "EigenSystem.h"

#include "MooseApp.h"

template<>
InputParameters validParams<EigenExecutionerBase>()
{
  InputParameters params = validParams<Executioner>();
  params.addRequiredParam<PostprocessorName>("bx_norm", "To evaluate |Bx| for the eigenvalue");
  params.addParam<PostprocessorName>("xdiff", "To evaluate |x-x_previous| for power iterations");
  params.addParam<PostprocessorName>("normalization", "To evaluate |x| for normalization");
  params.addParam<Real>("normal_factor", "Normalize x to make |x| equal to this factor");
  params.addParam<bool>("auto_initialization", true, "True to ask the solver to set initial");
  params.addParam<Real>("time", 0.0, "System time");
  params.addParam<bool>("output_on_final", false, "True to disable all the intemediate exodus outputs");

  params.addPrivateParam<bool>("_eigen", true);
  return params;
}

EigenExecutionerBase::EigenExecutionerBase(const std::string & name, InputParameters parameters)
    :Executioner(name, parameters),
     _problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem", "This might happen if you don't have a mesh")),
     _eigen_sys(static_cast<EigenSystem &>(_problem.getNonlinearSystem())),
     _eigenvalue(_problem.parameters().set<Real>("eigenvalue")), // used for storing the eigenvalue
     _source_integral(getPostprocessorValue("bx_norm")),
     _source_integral_old(getPostprocessorValueOld("bx_norm")),
     _solution_diff(isParamValid("xdiff") ? &getPostprocessorValue("xdiff") : NULL),
     _normalization(isParamValid("normalization") ? getPostprocessorValue("normalization")
                    : getPostprocessorValue("bx_norm")) // use |Bx| for normalization by default
{
  _eigenvalue = 1.0;

  // EigenKernel needs this postprocessor
  _problem.parameters().set<PostprocessorName>("eigen_postprocessor")
    = getParam<PostprocessorName>("bx_norm");

  //FIXME: currently we have to use old and older solution vectors for power iteration.
  //       We will need 'step' in the future.
  _problem.transient(true);

  {
    // No time integrator for eigenvalue problem
    std::string ti_str = "SteadyState";
    InputParameters params = _app.getFactory().getValidParams(ti_str);
    _problem.addTimeIntegrator(ti_str, "ti", params);
  }

  // we want to tell the App about what our system time is (in case anyone else is interested).
  Real system_time = getParam<Real>("time");
  _app.setStartTime(system_time);

  // set the system time
  _problem.time() = system_time;
  _problem.timeOld() = system_time;

  // used for controlling screen print-out
  _problem.timeStep() = 0;
  _problem.dt() = 1.0;
}

EigenExecutionerBase::~EigenExecutionerBase()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
EigenExecutionerBase::init()
{
  if (_app.isRecovering())
  {
    Moose::out<<"\nCannot recover eigenvalue solves!\nExiting...\n"<<std::endl;
    return;
  }

  checkIntegrity();
  _eigen_sys.buildSystemDoFIndices(EigenSystem::EIGEN);

  if (getParam<bool>("auto_initialization"))
  {
    // Initialize the solution of the eigen variables
    // Note: initial conditions will override this if there is any by _problem.initialSetup()
    _eigen_sys.initSystemSolution(EigenSystem::EIGEN, 1.0);
  }
  _problem.initialSetup();
  if (_source_integral==0.0) mooseError("|Bx| = 0!");
  _eigen_sys.initSystemSolutionOld(EigenSystem::EIGEN, 0.0);

  // check when the postprocessors are evaluated
  _bx_execflag = _problem.getUserObject<UserObject>(getParam<PostprocessorName>("bx_norm")).execFlag();
  if (_solution_diff)
    _xdiff_execflag = _problem.getUserObject<UserObject>(getParam<PostprocessorName>("xdiff")).execFlag();
  else
    _xdiff_execflag = EXEC_TIMESTEP;
  if (isParamValid("normalization"))
    _norm_execflag = _problem.getUserObject<UserObject>(getParam<PostprocessorName>("normalization")).execFlag();
  else
    _norm_execflag = _bx_execflag;

  // normalize solution to make |Bx|=_eigenvalue, _eigenvalue at this point has the initialized value
  makeBXConsistent(_eigenvalue);

  /* a time step check point */
  _problem.onTimestepEnd();

  Moose::setup_perf_log.push("Output Initial Condition","Setup");

  // Write the initial.
  // Note: We need to tempararily change the system time to make the output system work properly.
  _problem.timeStep() = 0;
  Real t = _problem.time();
  _problem.time() = _problem.timeStep();
  _output_warehouse.outputInitial();
  _problem.time() = t;
  Moose::setup_perf_log.pop("Output Initial Condition","Setup");
}

void
EigenExecutionerBase::makeBXConsistent(Real k)
{
  Real consistency_tolerance = 1e-10;

  // Scale the solution so that the postprocessor is equal to one.
  // We have a fix point loop here, in case the postprocessor is a nonlinear function of the scaling factor.
  // FIXME: We have assumed this loop always converges.
  while (std::fabs(k-_source_integral)>consistency_tolerance*std::fabs(k))
  {
    // On the first time entering, the _source_integral has been updated properly in FEProblem::initialSetup()
    _eigen_sys.scaleSystemSolution(EigenSystem::EIGEN, k/_source_integral);
    // update all aux variables
    for (unsigned int i=0; i<Moose::exec_types.size(); i++)
    {
      // EXEC_CUSTOM is special, should be treated only by specifically designed executioners.
      if (Moose::exec_types[i]==EXEC_CUSTOM) continue;
      _problem.computeUserObjects(Moose::exec_types[i], UserObjectWarehouse::PRE_AUX);
      _problem.computeAuxiliaryKernels(Moose::exec_types[i]);
      _problem.computeUserObjects(Moose::exec_types[i], UserObjectWarehouse::POST_AUX);
    }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(10) << _source_integral;
    Moose::out << " |Bx_0| = " << ss.str() << std::endl;
  }
}

void
EigenExecutionerBase::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation
  if (_eigen_sys.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state eigenvalue simulation");
}

void
EigenExecutionerBase::addRealParameterReporter(const std::string & param_name)
{
  InputParameters params = _app.getFactory().getValidParams("ProblemRealParameter");
  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep";
  params.set<MooseEnum>("execute_on") = execute_options;
  params.set<std::string>("param_name") = param_name;
  _problem.addPostprocessor("ProblemRealParameter", param_name, params);
}

void
EigenExecutionerBase::inversePowerIteration(unsigned int min_iter,
                                            unsigned int max_iter,
                                            Real pfactor,
                                            bool cheb_on,
                                            Real tol_eig,
                                            Real tol_x,
                                            bool echo,
                                            bool output_convergence,
                                            Real time_base,
                                            Real & k,
                                            Real & initial_res)
{
  mooseAssert(max_iter>=min_iter, "Maximum number of power iterations must be greater than or equal to its minimum");
  mooseAssert(pfactor>0.0, "Invaid linear convergence tolerance");
  mooseAssert(tol_eig>0.0, "Invalid eigenvalue tolerance");
  mooseAssert(tol_x>0.0, "Invalid solution norm tolerance");
  if ( _bx_execflag != EXEC_TIMESTEP && _bx_execflag != EXEC_RESIDUAL)
    mooseError("rhs postprocessor for the power method has to be executed on timestep or residual");
  if ( _xdiff_execflag != EXEC_TIMESTEP && _xdiff_execflag != EXEC_RESIDUAL)
    mooseError("xdiff postprocessor for the power method has to be executed on timestep or residual");

  // not perform any iteration when max_iter==0
  if (max_iter==0) return;

  // turn off nonlinear flag so that RHS kernels opterate on previous solutions
  _eigen_sys.eigenKernelOnOld();

  // FIXME: currently power iteration use old and older solutions,
  // so save old and older solutions before they are changed by the power iteration
  _eigen_sys.saveOldSolutions();

  // _es.parameters.print(Moose::out);
  // save solver control parameters to be modified by the power iteration
  Real tol1 = _problem.es().parameters.get<Real> ("linear solver tolerance");
  unsigned int num1 = _problem.es().parameters.get<unsigned int>("nonlinear solver maximum iterations");

  // every power iteration is a linear solve, so set nonlinear iteration number to one
  _problem.es().parameters.set<Real> ("linear solver tolerance") = pfactor;
  _problem.es().parameters.set<unsigned int>("nonlinear solver maximum iterations") = 1;

  if (echo)
  {
    Moose::out << std::endl;
    Moose::out << " Power iterations starts" << std::endl;
    Moose::out << " ________________________________________________________________________________ " << std::endl;
  }

  // some iteration variables
  Real k_old = 0.0;

  std::vector<Real> keff_history;
  std::vector<Real> diff_history;

  unsigned int iter = 0;

  // power iteration loop...
  // Note: |Bx|/k will stay constant one!
  makeBXConsistent(k);
  while (true)
  {
    if (echo)  Moose::out << " Power iteration= "<< iter << std::endl;

    // important: solutions of aux system is also copied
    _problem.advanceState();
    k_old = k;

    // FIXME: timestep needs to be changed to step
    _problem.onTimestepBegin(); // this will copy postprocessors to old
    _problem.timestepSetup();
    _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::PRE_AUX);
    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);
    _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::POST_AUX);

    preIteration();
    _problem.solve();
    postIteration();

    // FIXME: timestep needs to be changed to step
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
    _problem.onTimestepEnd();

    // save the initial residual
    if (iter==0) initial_res = _eigen_sys._initial_residual;

    // update eigenvalue
    k = k_old * _source_integral / _source_integral_old;

    // synchronize _eigenvalue with |Bx| for output purpose
    // Note: if using MOOSE output system, eigenvalue output will be one iteration behind.
    //       Also this will affect EigenKernels with eigen=false.
    _eigenvalue = k;

    if (echo && (!output_convergence))
    {
      // output on screen the convergence history only when we want to and MOOSE output system is not used
      keff_history.push_back(k);
      if (_solution_diff) diff_history.push_back(*_solution_diff);

      std::ios_base::fmtflags flg = Moose::out.flags();
      std::streamsize pcs = Moose::out.precision();
      if (_solution_diff)
      {
        Moose::out << std::endl;
        Moose::out << " +================+=====================+=====================+\n";
        Moose::out << " | iteration      | eigenvalue          | solution_difference |\n";
        Moose::out << " +================+=====================+=====================+\n";
        unsigned int j = 0;
        if (keff_history.size()>10)
        {
          Moose::out << " :                :                     :                     :\n";
          j = keff_history.size()-10;
        }
        for (; j<keff_history.size(); j++)
          Moose::out << " | " << std::setw(14) << j
                     << " | " << std::setw(19) << std::scientific << std::setprecision(8) << keff_history[j]
                     << " | " << std::setw(19) << std::scientific << std::setprecision(8) << diff_history[j]
                     << " |\n";
        Moose::out << " +================+=====================+=====================+\n" << std::flush;
        Moose::out << std::endl;
      }
      else
      {
        Moose::out << std::endl;
        Moose::out << " +================+=====================+\n";
        Moose::out << " | iteration      | eigenvalue          |\n";
        Moose::out << " +================+=====================+\n";
        unsigned int j = 0;
        if (keff_history.size()>10)
        {
          Moose::out << " :                :                     :\n";
          j = keff_history.size()-10;
        }
        for (; j<keff_history.size(); j++)
          Moose::out << " | " << std::setw(14) << j
                     << " | " << std::setw(19) << std::scientific << std::setprecision(8) << keff_history[j]
                     << " |\n";
        Moose::out << " +================+=====================+\n" << std::flush;
        Moose::out << std::endl;
      }
      Moose::out.flags(flg);
      Moose::out.precision(pcs);
    }

    // increment iteration number here
    iter++;

    if (cheb_on)
    {
      chebyshev(iter);
      if (echo)
        Moose::out << " Chebyshev step: " << chebyshev_parameters.icheb << std::endl;
    }

    if (echo)
      Moose::out << " ________________________________________________________________________________ "
                 << std::endl;

    // not perform any convergence check when number of iterations is less than min_iter
    if (iter>=min_iter)
    {
      // no need to check convergence of the last iteration
      if (iter!=max_iter)
      {
        bool converged = true;
        Real keff_error = fabs(k_old-k)/k;
        if (keff_error>tol_eig) converged = false;
        if (_solution_diff)
          if (*_solution_diff > tol_x) converged = false;
        if (converged) break;
      }
      else
        break;
    }

    // use output system to dump iteration history
    if (output_convergence)
    {
      // we need to tempararily change system time to obtain the right output
      // FIXME: if 'step' capability is available, we will not need to do this.
      Real t = _problem.time();
      _problem.time() = time_base + Real(iter)/max_iter;
      _output_warehouse.outputStep();
      _problem.time() = t;
    }
  }

  // restore parameters changed by the executioner
  _problem.es().parameters.set<Real> ("linear solver tolerance") = tol1;
  _problem.es().parameters.set<unsigned int>("nonlinear solver maximum iterations") = num1;

  //FIXME: currently power iteration use old and older solutions, so restore them
  _eigen_sys.restoreOldSolutions();
}

void
EigenExecutionerBase::preIteration()
{
}

void
EigenExecutionerBase::postIteration()
{
}

void
EigenExecutionerBase::postExecute()
{

  if (!getParam<bool>("output_on_final"))
  {
    _problem.timeStep()++;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _output_warehouse.outputStep();
    _problem.time() = t;
  }

  Real s = 1.0;
  if (_norm_execflag==EXEC_CUSTOM)
  {
    Moose::out << " Cannot let the normalization postprocessor on custom." << std::endl;
    Moose::out << " Normalization is abandoned!" << std::endl;
  }
  else
  {
    s = normalizeSolution(_norm_execflag!=EXEC_TIMESTEP && _norm_execflag!=EXEC_RESIDUAL);
    if (std::fabs(s-1.0)>std::numeric_limits<Real>::epsilon())
      Moose::out << " Solution is rescaled with factor " << s << " for normalization!" << std::endl;
  }

  if (getParam<bool>("output_on_final") || std::fabs(s-1.0)>std::numeric_limits<Real>::epsilon())
  {
    _problem.timeStep()++;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _output_warehouse.outputStep();
    _problem.time() = t;
  }
}

Real
EigenExecutionerBase::normalizeSolution(bool force)
{
  if (force)
  {
    _problem.computeUserObjects(_norm_execflag);
    _problem.computeAuxiliaryKernels(_norm_execflag);
    _problem.computeUserObjects(_norm_execflag, UserObjectWarehouse::POST_AUX);
  }

  Real factor;
  if (isParamValid("normal_factor"))
    factor = getParam<Real>("normal_factor");
  else
    factor = _eigenvalue;
  Real scaling = factor/_normalization;

  if (scaling != 1.0)
  {
    //FIXME: we assume linear scaling here!
    _eigen_sys.scaleSystemSolution(EigenSystem::EIGEN, scaling);
    // update all aux variables and user objects
    for (unsigned int i=0; i<Moose::exec_types.size(); i++)
    {
      // EXEC_CUSTOM is special, should be treated only by specifically designed executioners.
      if (Moose::exec_types[i]==EXEC_CUSTOM) continue;
      _problem.computeUserObjects(Moose::exec_types[i], UserObjectWarehouse::PRE_AUX);
      _problem.computeAuxiliaryKernels(Moose::exec_types[i]);
      _problem.computeUserObjects(Moose::exec_types[i], UserObjectWarehouse::POST_AUX);
    }
  }
  return scaling;
}

void
EigenExecutionerBase::printEigenvalue()
{
  std::ostringstream ss;
  ss << std::endl;
  ss << "******************************************************* " << std::endl;
  ss << " Eigenvalue = " << std::fixed << std::setprecision(10) << _eigenvalue << std::endl;
  ss << "******************************************************* " << std::endl;

  Moose::out << ss.str();
}

EigenExecutionerBase::Chebyshev_Parameters::Chebyshev_Parameters ()
  :
  n_iter(50),
  fsmooth(2),
  finit(6),
  lgac(0),
  icheb(0),
  icho(0)
{}

void
EigenExecutionerBase::Chebyshev_Parameters::reinit ()
{
  finit   = 6;
  lgac    = 0;
  icho    = 0;
  icheb   = 0;
}

void
EigenExecutionerBase::chebyshev(unsigned int iter)
{
  if (chebyshev_parameters.lgac==0)
  {
    if (chebyshev_parameters.icho==0)
      chebyshev_parameters.ratio = *_solution_diff / chebyshev_parameters.flux_error_norm_old;
    else
    {
      chebyshev_parameters.ratio = chebyshev_parameters.ratio_new;
      chebyshev_parameters.icho = 0;
    }

    if (iter > chebyshev_parameters.finit &&
        chebyshev_parameters.ratio>=0.4 &&
        chebyshev_parameters.ratio<=1)
    {
      chebyshev_parameters.lgac = 1;
      chebyshev_parameters.icheb = 1;
      chebyshev_parameters.error_begin = *_solution_diff;
      chebyshev_parameters.iter_begin = iter;
      double alp = 2/(2-chebyshev_parameters.ratio);
      std::vector<double> coef(2);
      coef[0] = alp;
      coef[1] = 1-alp;
      _eigen_sys.combineSystemSolution(EigenSystem::EIGEN, coef);
      _problem.computeUserObjects(EXEC_RESIDUAL, UserObjectWarehouse::PRE_AUX);
      _problem.computeAuxiliaryKernels(EXEC_RESIDUAL);
      _problem.computeUserObjects(EXEC_RESIDUAL, UserObjectWarehouse::POST_AUX);
      _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
      _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
      _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
      _eigenvalue = _source_integral;
    }
  }
  else
  {
    chebyshev_parameters.icheb++;
    double gamma = acosh(2/chebyshev_parameters.ratio-1);
    double alp = 4/chebyshev_parameters.ratio*std::cosh((chebyshev_parameters.icheb-1)*gamma)
      /std::cosh(chebyshev_parameters.icheb*gamma);
    double beta = (1-chebyshev_parameters.ratio/2)*alp - 1;
/*  if (iter<int(chebyshev_parameters.iter_begin+chebyshev_parameters.n_iter))
    {
      std::vector<double> coef(3);
      coef[0] = alp;
      coef[1] = 1-alp+beta;
      coef[2] = -beta;
      _eigen_sys.combineSystemSolution(NonlinearSystem::EIGEN, coef);
    }
    else
    {*/
      double gamma_new = (*_solution_diff/chebyshev_parameters.error_begin)*
        (std::cosh((chebyshev_parameters.icheb-1)*acosh(2/chebyshev_parameters.ratio-1)));
      if (gamma_new<1.0) gamma_new = 1.0;

      chebyshev_parameters.ratio_new = chebyshev_parameters.ratio/2*
        (std::cosh(acosh(gamma_new) / (chebyshev_parameters.icheb-1))+1);
      if (gamma_new>1.01)
      {
        chebyshev_parameters.lgac = 0;
//      chebyshev_parameters.icheb = 0;
//      if (chebyshev_parameters.icheb>30)
//      {
        if (chebyshev_parameters.icheb>0)
        {
          chebyshev_parameters.icho = 1;
          chebyshev_parameters.finit = iter;
        }
        else
        {
          chebyshev_parameters.icho = 0;
          chebyshev_parameters.finit = iter + chebyshev_parameters.fsmooth;
        }
      }
      else
      {
        std::vector<double> coef(3);
        coef[0] = alp;
        coef[1] = 1-alp+beta;
        coef[2] = -beta;
        _eigen_sys.combineSystemSolution(EigenSystem::EIGEN, coef);
        _problem.computeUserObjects(EXEC_RESIDUAL, UserObjectWarehouse::PRE_AUX);
        _problem.computeAuxiliaryKernels(EXEC_RESIDUAL);
        _problem.computeUserObjects(EXEC_RESIDUAL, UserObjectWarehouse::POST_AUX);
        _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
        _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
        _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
        _eigenvalue = _source_integral;
      }
//    }
  }
  chebyshev_parameters.flux_error_norm_old = *_solution_diff;
}

void
EigenExecutionerBase::nonlinearSolve(Real rel_tol, Real abs_tol, Real pfactor, Real & k)
{
  PostprocessorName bxp = getParam<PostprocessorName>("bx_norm");
  if ( _problem.getUserObject<UserObject>(bxp).execFlag() != EXEC_RESIDUAL)
    mooseError("rhs postprocessor for the nonlinear eigenvalue solve must be executed on residual");
  makeBXConsistent(k);

  // turn on nonlinear flag so that RHS kernels opterate on the current solutions
  _eigen_sys.eigenKernelOnCurrent();

  // set nonlinear solver controls
  Real tol1 = _problem.es().parameters.get<Real> ("nonlinear solver absolute residual tolerance");
  Real tol2 = _problem.es().parameters.get<Real> ("linear solver tolerance");
  Real tol3 = _problem.es().parameters.get<Real> ("nonlinear solver relative residual tolerance");

  _problem.es().parameters.set<Real> ("nonlinear solver absolute residual tolerance") = abs_tol;
  _problem.es().parameters.set<Real> ("nonlinear solver relative residual tolerance") = rel_tol;
  _problem.es().parameters.set<Real> ("linear solver tolerance") = pfactor;

  // call nonlinear solve
  _problem.solve();

  k = _source_integral;
  _eigenvalue = k;

  _problem.es().parameters.set<Real> ("nonlinear solver absolute residual tolerance") = tol1;
  _problem.es().parameters.set<Real> ("linear solver tolerance") = tol2;
  _problem.es().parameters.set<Real> ("nonlinear solver relative residual tolerance") = tol3;
}
