//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenExecutionerBase.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseEigenSystem.h"
#include "UserObject.h"

InputParameters
EigenExecutionerBase::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for eigenvalue problems.");

  params += FEProblemSolve::validParams();

  params.addRequiredParam<PostprocessorName>("bx_norm", "To evaluate |Bx| for the eigenvalue");
  params.addParam<PostprocessorName>("normalization", "To evaluate |x| for normalization");
  params.addParam<Real>("normal_factor", "Normalize x to make |x| equal to this factor");
  params.addParam<bool>(
      "output_before_normalization", true, "True to output a step before normalization");
  params.addParam<bool>("auto_initialization", true, "True to ask the solver to set initial");
  params.addParam<Real>("time", 0.0, "System time");

  params.addPrivateParam<bool>("_eigen", true);

  params.addParamNamesToGroup("normalization normal_factor output_before_normalization",
                              "Normalization");
  params.addParamNamesToGroup("auto_initialization time", "Advanced");

  params.addParam<Real>("k0", 1.0, "Initial guess of the eigenvalue");

  params.addPrivateParam<bool>("_eigen", true);

  return params;
}

const Real &
EigenExecutionerBase::eigenvalueOld()
{
  return _source_integral_old;
}

EigenExecutionerBase::EigenExecutionerBase(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _eigen_sys(static_cast<MooseEigenSystem &>(_problem.getNonlinearSystemBase())),
    _feproblem_solve(*this),
    _eigenvalue(addAttributeReporter("eigenvalue", getParam<Real>("k0"))),
    _source_integral(getPostprocessorValue("bx_norm")),
    _source_integral_old(1),
    _normalization(isParamValid("normalization")
                       ? getPostprocessorValue("normalization")
                       : getPostprocessorValue("bx_norm")) // use |Bx| for normalization by default
{
  // FIXME: currently we have to use old and older solution vectors for power iteration.
  //       We will need 'step' in the future.
  _problem.transient(true);
  _problem.getNonlinearSystemBase().needSolutionState(2);
  _fe_problem.getAuxiliarySystem().needSolutionState(2);

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

void
EigenExecutionerBase::init()
{
  checkIntegrity();
  _eigen_sys.buildSystemDoFIndices(MooseEigenSystem::EIGEN);

  if (getParam<bool>("auto_initialization"))
  {
    // Initialize the solution of the eigen variables
    // Note: initial conditions will override this if there is any by _problem.initialSetup()
    _eigen_sys.initSystemSolution(MooseEigenSystem::EIGEN, 1.0);
  }
  _problem.initialSetup();
  _eigen_sys.initSystemSolutionOld(MooseEigenSystem::EIGEN, 0.0);

  // check when the postprocessors are evaluated
  const ExecFlagEnum & bx_exec =
      _problem.getUserObject<UserObject>(getParam<PostprocessorName>("bx_norm")).getExecuteOnEnum();
  if (!bx_exec.contains(EXEC_LINEAR))
    mooseError("Postprocessor " + getParam<PostprocessorName>("bx_norm") +
               " requires execute_on = 'linear'");

  if (isParamValid("normalization"))
    _norm_exec = _problem.getUserObject<UserObject>(getParam<PostprocessorName>("normalization"))
                     .getExecuteOnEnum();
  else
    _norm_exec = bx_exec;

  // check if _source_integral has been evaluated during initialSetup()
  if (!bx_exec.contains(EXEC_INITIAL))
    _problem.execute(EXEC_LINEAR);

  if (_source_integral == 0.0)
    mooseError("|Bx| = 0!");

  // normalize solution to make |Bx|=_eigenvalue, _eigenvalue at this point has the initialized
  // value
  makeBXConsistent(_eigenvalue);

  if (_problem.getDisplacedProblem() != NULL)
    _problem.getDisplacedProblem()->syncSolutions();

  /* a time step check point */
  _problem.onTimestepEnd();
}

void
EigenExecutionerBase::makeBXConsistent(Real k)
{
  Real consistency_tolerance = 1e-10;

  // Scale the solution so that the postprocessor is equal to k.
  // Note: all dependent objects of k must be evaluated on linear!
  // We have a fix point loop here, in case the postprocessor is a nonlinear function of the scaling
  // factor.
  // FIXME: We have assumed this loop always converges.
  while (std::fabs(k - _source_integral) > consistency_tolerance * std::fabs(k))
  {
    // On the first time entering, the _source_integral has been updated properly in
    // FEProblemBase::initialSetup()
    _eigen_sys.scaleSystemSolution(MooseEigenSystem::EIGEN, k / _source_integral);
    _problem.execute(EXEC_LINEAR);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(10) << _source_integral;
    _console << "\n|Bx| = " << ss.str() << std::endl;
  }
}

void
EigenExecutionerBase::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation
  if (_eigen_sys.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state eigenvalue simulation");
  if (!_eigen_sys.containsEigenKernel())
    mooseError("You have not specified any eigen kernels in your eigenvalue simulation");
}

bool
EigenExecutionerBase::inversePowerIteration(unsigned int min_iter,
                                            unsigned int max_iter,
                                            Real l_rtol,
                                            bool cheb_on,
                                            Real tol_eig,
                                            bool echo,
                                            PostprocessorName xdiff,
                                            Real tol_x,
                                            Real & k,
                                            Real & initial_res)
{
  mooseAssert(max_iter >= min_iter,
              "Maximum number of power iterations must be greater than or equal to its minimum");
  mooseAssert(l_rtol > 0.0, "Invaid linear convergence tolerance");
  mooseAssert(tol_eig > 0.0, "Invalid eigenvalue tolerance");
  mooseAssert(tol_x > 0.0, "Invalid solution norm tolerance");

  // obtain the solution diff
  const PostprocessorValue * solution_diff = NULL;
  if (!xdiff.empty())
  {
    solution_diff = &_problem.getPostprocessorValueByName(xdiff);
    const ExecFlagEnum & xdiff_exec = _problem.getUserObject<UserObject>(xdiff).getExecuteOnEnum();
    if (!xdiff_exec.contains(EXEC_LINEAR))
      mooseError("Postprocessor " + xdiff + " requires execute_on = 'linear'");
  }

  // not perform any iteration when max_iter==0
  if (max_iter == 0)
    return true;

  // turn off nonlinear flag so that RHS kernels opterate on previous solutions
  _eigen_sys.eigenKernelOnOld();

  // FIXME: currently power iteration use old and older solutions,
  // so save old and older solutions before they are changed by the power iteration
  _problem.saveOldSolutions();
  if (_problem.getDisplacedProblem() != NULL)
    _problem.getDisplacedProblem()->saveOldSolutions();

  // save solver control parameters to be modified by the power iteration
  Real tol1 = _problem.es().parameters.get<Real>("linear solver tolerance");
  unsigned int num1 =
      _problem.es().parameters.get<unsigned int>("nonlinear solver maximum iterations");
  Real tol2 = _problem.es().parameters.get<Real>("nonlinear solver relative residual tolerance");

  // every power iteration is a linear solve, so set nonlinear iteration number to one
  _problem.es().parameters.set<Real>("linear solver tolerance") = l_rtol;
  // disable nonlinear convergence check
  _problem.es().parameters.set<unsigned int>("nonlinear solver maximum iterations") = 1;
  _problem.es().parameters.set<Real>("nonlinear solver relative residual tolerance") = 1 - 1e-8;

  if (echo)
  {
    _console << '\n';
    _console << " Power iterations starts\n";
    _console << " ________________________________________________________________________________ "
             << std::endl;
  }

  // some iteration variables
  Chebyshev_Parameters chebyshev_parameters;

  std::vector<Real> keff_history;
  std::vector<Real> diff_history;

  bool converged;

  unsigned int iter = 0;

  // power iteration loop...
  // Note: |Bx|/k will stay constant one!
  makeBXConsistent(k);
  while (true)
  {
    if (echo)
      _console << " Power iteration= " << iter << std::endl;

    // Important: we do not call _problem.advanceState() because we do not
    // want to overwrite the old postprocessor values and old material
    // properties in stateful materials.
    _problem.getNonlinearSystemBase().copyOldSolutions();
    _problem.getAuxiliarySystem().copyOldSolutions();
    if (_problem.getDisplacedProblem() != NULL)
    {
      _problem.getDisplacedProblem()->nlSys().copyOldSolutions();
      _problem.getDisplacedProblem()->auxSys().copyOldSolutions();
    }

    Real k_old = k;
    _source_integral_old = _source_integral;

    preIteration();
    _problem.solve();
    converged = _problem.converged();
    if (!converged)
      break;
    postIteration();

    // save the initial residual
    if (iter == 0)
      initial_res = _eigen_sys._initial_residual_before_preset_bcs;

    // update eigenvalue
    k = k_old * _source_integral / _source_integral_old;
    _eigenvalue = k;

    if (echo)
    {
      // output on screen the convergence history only when we want to and MOOSE output system is
      // not used
      keff_history.push_back(k);
      if (solution_diff)
        diff_history.push_back(*solution_diff);

      std::stringstream ss;
      if (solution_diff)
      {
        ss << '\n';
        ss << " +================+=====================+=====================+\n";
        ss << " | iteration      | eigenvalue          | solution_difference |\n";
        ss << " +================+=====================+=====================+\n";
        unsigned int j = 0;
        if (keff_history.size() > 10)
        {
          ss << " :                :                     :                     :\n";
          j = keff_history.size() - 10;
        }
        for (; j < keff_history.size(); j++)
          ss << " | " << std::setw(14) << j << " | " << std::setw(19) << std::scientific
             << std::setprecision(8) << keff_history[j] << " | " << std::setw(19) << std::scientific
             << std::setprecision(8) << diff_history[j] << " |\n";
        ss << " +================+=====================+=====================+\n" << std::flush;
      }
      else
      {
        ss << '\n';
        ss << " +================+=====================+\n";
        ss << " | iteration      | eigenvalue          |\n";
        ss << " +================+=====================+\n";
        unsigned int j = 0;
        if (keff_history.size() > 10)
        {
          ss << " :                :                     :\n";
          j = keff_history.size() - 10;
        }
        for (; j < keff_history.size(); j++)
          ss << " | " << std::setw(14) << j << " | " << std::setw(19) << std::scientific
             << std::setprecision(8) << keff_history[j] << " |\n";
        ss << " +================+=====================+\n" << std::flush;
        ss << std::endl;
      }
      _console << ss.str();
    }

    // increment iteration number here
    iter++;

    if (cheb_on)
    {
      chebyshev(chebyshev_parameters, iter, solution_diff);
      if (echo)
        _console << " Chebyshev step: " << chebyshev_parameters.icheb << std::endl;
    }

    if (echo)
      _console
          << " ________________________________________________________________________________ "
          << std::endl;

    // not perform any convergence check when number of iterations is less than min_iter
    if (iter >= min_iter)
    {
      // no need to check convergence of the last iteration
      if (iter != max_iter)
      {
        Real keff_error = fabs(k_old - k) / k;
        if (keff_error > tol_eig)
          converged = false;
        if (solution_diff)
          if (*solution_diff > tol_x)
            converged = false;
        if (converged)
          break;
      }
      else
      {
        converged = false;
        break;
      }
    }
  }

  // restore parameters changed by the executioner
  _problem.es().parameters.set<Real>("linear solver tolerance") = tol1;
  _problem.es().parameters.set<unsigned int>("nonlinear solver maximum iterations") = num1;
  _problem.es().parameters.set<Real>("nonlinear solver relative residual tolerance") = tol2;

  // FIXME: currently power iteration use old and older solutions, so restore them
  _problem.restoreOldSolutions();
  if (_problem.getDisplacedProblem() != NULL)
    _problem.getDisplacedProblem()->restoreOldSolutions();

  return converged;
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
  if (getParam<bool>("output_before_normalization"))
  {
    _problem.timeStep()++;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _problem.outputStep(EXEC_TIMESTEP_END);
    _problem.time() = t;
  }

  Real s = 1.0;
  if (_norm_exec.contains(EXEC_CUSTOM))
  {
    _console << " Cannot let the normalization postprocessor on custom.\n";
    _console << " Normalization is abandoned!" << std::endl;
  }
  else
  {
    bool force = _norm_exec.contains(EXEC_TIMESTEP_END) || _norm_exec.contains(EXEC_LINEAR);
    s = normalizeSolution(force);
    if (!MooseUtils::absoluteFuzzyEqual(s, 1.0))
      _console << " Solution is rescaled with factor " << s << " for normalization!" << std::endl;
  }

  if ((!getParam<bool>("output_before_normalization")) || !MooseUtils::absoluteFuzzyEqual(s, 1.0))
  {
    _problem.timeStep()++;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _problem.outputStep(EXEC_TIMESTEP_END);
    _problem.time() = t;
  }

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiApps(EXEC_FINAL);
    _problem.execute(EXEC_FINAL);
    _problem.outputStep(EXEC_FINAL);
  }
}

Real
EigenExecutionerBase::normalizeSolution(bool force)
{
  if (force)
    _problem.execute(EXEC_INITIAL);

  Real factor;
  if (isParamValid("normal_factor"))
    factor = getParam<Real>("normal_factor");
  else
    factor = _eigenvalue;
  Real scaling = factor / _normalization;

  if (!MooseUtils::absoluteFuzzyEqual(scaling, 1.0))
  {
    // FIXME: we assume linear scaling here!
    _eigen_sys.scaleSystemSolution(MooseEigenSystem::EIGEN, scaling);
    // update all aux variables and user objects

    for (const ExecFlagType & flag : _app.getExecuteOnEnum().items())
      _problem.execute(flag);
  }
  return scaling;
}

void
EigenExecutionerBase::printEigenvalue()
{
  std::ostringstream ss;
  ss << '\n';
  ss << "*******************************************************\n";
  ss << " Eigenvalue = " << std::fixed << std::setprecision(10) << _eigenvalue << '\n';
  ss << "*******************************************************";

  _console << ss.str() << std::endl;
}

EigenExecutionerBase::Chebyshev_Parameters::Chebyshev_Parameters()
  : n_iter(50), fsmooth(2), finit(6), lgac(0), icheb(0), flux_error_norm_old(1), icho(0)
{
}

void
EigenExecutionerBase::Chebyshev_Parameters::reinit()
{
  finit = 6;
  lgac = 0;
  icheb = 0;
  flux_error_norm_old = 1;
  icho = 0;
}

void
EigenExecutionerBase::chebyshev(Chebyshev_Parameters & chebyshev_parameters,
                                unsigned int iter,
                                const PostprocessorValue * solution_diff)
{
  if (!solution_diff)
    mooseError("solution diff is required for Chebyshev acceleration");

  if (chebyshev_parameters.lgac == 0)
  {
    if (chebyshev_parameters.icho == 0)
      chebyshev_parameters.ratio = *solution_diff / chebyshev_parameters.flux_error_norm_old;
    else
    {
      chebyshev_parameters.ratio = chebyshev_parameters.ratio_new;
      chebyshev_parameters.icho = 0;
    }

    if (iter > chebyshev_parameters.finit && chebyshev_parameters.ratio >= 0.4 &&
        chebyshev_parameters.ratio <= 1)
    {
      chebyshev_parameters.lgac = 1;
      chebyshev_parameters.icheb = 1;
      chebyshev_parameters.error_begin = *solution_diff;
      chebyshev_parameters.iter_begin = iter;
      double alp = 2 / (2 - chebyshev_parameters.ratio);
      std::vector<double> coef(2);
      coef[0] = alp;
      coef[1] = 1 - alp;
      _eigen_sys.combineSystemSolution(MooseEigenSystem::EIGEN, coef);
      _problem.execute(EXEC_LINEAR);
      _eigenvalue = _source_integral;
    }
  }
  else
  {
    chebyshev_parameters.icheb++;
    double gamma = acosh(2 / chebyshev_parameters.ratio - 1);
    double alp = 4 / chebyshev_parameters.ratio *
                 std::cosh((chebyshev_parameters.icheb - 1) * gamma) /
                 std::cosh(chebyshev_parameters.icheb * gamma);
    double beta = (1 - chebyshev_parameters.ratio / 2) * alp - 1;
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
    double gamma_new =
        (*solution_diff / chebyshev_parameters.error_begin) *
        (std::cosh((chebyshev_parameters.icheb - 1) * acosh(2 / chebyshev_parameters.ratio - 1)));
    if (gamma_new < 1.0)
      gamma_new = 1.0;

    chebyshev_parameters.ratio_new =
        chebyshev_parameters.ratio / 2 *
        (std::cosh(acosh(gamma_new) / (chebyshev_parameters.icheb - 1)) + 1);
    if (gamma_new > 1.01)
    {
      chebyshev_parameters.lgac = 0;
      //      chebyshev_parameters.icheb = 0;
      //      if (chebyshev_parameters.icheb>30)
      //      {
      if (chebyshev_parameters.icheb > 0)
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
      coef[1] = 1 - alp + beta;
      coef[2] = -beta;
      _eigen_sys.combineSystemSolution(MooseEigenSystem::EIGEN, coef);
      _problem.execute(EXEC_LINEAR);
      _eigenvalue = _source_integral;
    }
    //    }
  }
  chebyshev_parameters.flux_error_norm_old = *solution_diff;
}

bool
EigenExecutionerBase::nonlinearSolve(Real nl_rtol, Real nl_atol, Real l_rtol, Real & k)
{
  makeBXConsistent(k);

  // turn on nonlinear flag so that eigen kernels opterate on the current solutions
  _eigen_sys.eigenKernelOnCurrent();

  // set nonlinear solver controls
  Real tol1 = _problem.es().parameters.get<Real>("nonlinear solver absolute residual tolerance");
  Real tol2 = _problem.es().parameters.get<Real>("linear solver tolerance");
  Real tol3 = _problem.es().parameters.get<Real>("nonlinear solver relative residual tolerance");

  _problem.es().parameters.set<Real>("nonlinear solver absolute residual tolerance") = nl_atol;
  _problem.es().parameters.set<Real>("nonlinear solver relative residual tolerance") = nl_rtol;
  _problem.es().parameters.set<Real>("linear solver tolerance") = l_rtol;

  // call nonlinear solve
  _problem.solve();

  k = _source_integral;
  _eigenvalue = k;

  _problem.es().parameters.set<Real>("nonlinear solver absolute residual tolerance") = tol1;
  _problem.es().parameters.set<Real>("linear solver tolerance") = tol2;
  _problem.es().parameters.set<Real>("nonlinear solver relative residual tolerance") = tol3;

  return _problem.converged();
}
