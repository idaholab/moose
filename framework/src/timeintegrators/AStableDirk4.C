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

// MOOSE includes
#include "AStableDirk4.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "LStableDirk4.h"

template <>
InputParameters
validParams<AStableDirk4>()
{
  InputParameters params = validParams<TimeIntegrator>();
  params.addParam<bool>("safe_start", true, "If true, use LStableDirk4 to bootstrap this method.");
  return params;
}

AStableDirk4::AStableDirk4(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _gamma(0.5 + std::sqrt(3) / 3. * std::cos(libMesh::pi / 18.)),
    _safe_start(getParam<bool>("safe_start"))
{
  // Name the stage residuals "residual_stage1", "residual_stage2", etc.
  for (unsigned int stage = 0; stage < 3; ++stage)
  {
    std::ostringstream oss;
    oss << "residual_stage" << stage + 1;
    _stage_residuals[stage] = &(_nl.addVector(oss.str(), false, GHOSTED));
  }

  // Initialize parameters
  _c[0] = _gamma;
  _c[1] = .5;
  _c[2] = 1.0 - _gamma;

  _a[0][0] = _gamma;
  _a[1][0] = .5 - _gamma; /**/
  _a[1][1] = _gamma;
  _a[2][0] = 2. * _gamma;     /**/
  _a[2][1] = 1 - 4. * _gamma; /**/
  _a[2][2] = _gamma;

  _b[0] = 1. / (24. * (.5 - _gamma) * (.5 - _gamma));
  _b[1] = 1. - 1. / (12. * (.5 - _gamma) * (.5 - _gamma));
  _b[2] = _b[0];

  // If doing a _safe_start, construct the bootstrapping
  // TimeIntegrator.  Note that this method will also add
  // residual_stage vectors to the system, but since they have the
  // same name as the vectors we added, they won't be duplicated.
  if (_safe_start)
  {
    Factory & factory = _app.getFactory();
    InputParameters params = factory.getValidParams("LStableDirk4");

    // We need to set some parameters that are normally set in
    // FEProblemBase::addTimeIntegrator() to ensure that the
    // getCheckedPointerParam() sanity checking is happy.  This is why
    // constructing MOOSE objects "manually" is generally frowned upon.
    params.set<FEProblemBase *>("_fe_problem_base") = &_fe_problem;
    params.set<SystemBase *>("_sys") = &_sys;

    _bootstrap_method = factory.create<LStableDirk4>("LStableDirk4", name() + "_bootstrap", params);
  }
}

AStableDirk4::~AStableDirk4() {}

void
AStableDirk4::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postStep(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1. / _dt;
  _u_dot.close();
  _du_dot_du = 1. / _dt;
}

void
AStableDirk4::solve()
{
  if (_t_step == 1 && _safe_start)
    _bootstrap_method->solve();

  else
  {
    // Time at end of step
    Real time_old = _fe_problem.timeOld();

    // A for-loop would increment _stage too far, so we use an extra
    // loop counter.  The method has three stages and an update stage,
    // which we treat as just one more (special) stage in the implementation.
    for (unsigned int current_stage = 1; current_stage < 5; ++current_stage)
    {
      // Set the current stage value
      _stage = current_stage;

      // This ensures that all the Output objects in the OutputWarehouse
      // have had solveSetup() called, and sets the default solver
      // parameters for PETSc.
      _fe_problem.initPetscOutput();

      if (current_stage < 4)
      {
        _console << "Stage " << _stage << "\n";
        _fe_problem.time() = time_old + _c[_stage - 1] * _dt;
      }
      else
      {
        _console << "Update Stage.\n";
        _fe_problem.time() = time_old + _dt;
      }

      // Do the solve
      _fe_problem.getNonlinearSystemBase().system().solve();
    }
  }
}

void
AStableDirk4::postStep(NumericVector<Number> & residual)
{
  if (_t_step == 1 && _safe_start)
    _bootstrap_method->postStep(residual);

  else
  {
    // Error if _stage got messed up somehow.
    if (_stage > 4)
      mooseError("AStableDirk4::postStep(): Member variable _stage can only have values 1-4.");

    if (_stage < 4)
    {
      // In the standard RK notation, the residual of stage 1 of s is given by:
      //
      // R := M*(Y_i - y_n)/dt - \sum_{j=1}^s a_{ij} * f(t_n + c_j*dt, Y_j) = 0
      //
      // where:
      // .) M is the mass matrix
      // .) Y_i is the stage solution
      // .) dt is the timestep, and is accounted for in the _Re_time residual.
      // .) f are the "non-time" residuals evaluated for a given stage solution.
      // .) The minus signs are already "baked in" to the residuals and so do not appear below.

      // Store this stage's non-time residual.  We are calling operator=
      // here, and that calls close().
      *_stage_residuals[_stage - 1] = _Re_non_time;

      // Build up the residual for this stage.
      residual.add(1., _Re_time);
      for (unsigned int j = 0; j < _stage; ++j)
        residual.add(_a[_stage - 1][j], *_stage_residuals[j]);
      residual.close();
    }
    else
    {
      // The update step is a final solve:
      //
      // R := M*(y_{n+1} - y_n)/dt - \sum_{j=1}^s b_j * f(t_n + c_j*dt, Y_j) = 0
      //
      // We could potentially fold _b up into an extra row of _a and
      // just do one more stage, but I think handling it separately like
      // this is easier to understand and doesn't create too much code
      // repitition.
      residual.add(1., _Re_time);
      for (unsigned int j = 0; j < 3; ++j)
        residual.add(_b[j], *_stage_residuals[j]);
      residual.close();
    }
  }
}
