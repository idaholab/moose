//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FullSolveMultiApp.h"
#include "LayeredSideDiffusiveFluxAverage.h"
#include "Executioner.h"
#include "TransientBase.h"
#include "FEProblemBase.h"
#include "MaterialPropertyStorage.h"
#include "Console.h"

// libMesh
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", FullSolveMultiApp);

InputParameters
FullSolveMultiApp::validParams()
{
  InputParameters params = MultiApp::validParams();
  params.addClassDescription("Performs a complete simulation during each execution.");
  params.addParam<bool>(
      "keep_full_output_history",
      false,
      "Whether or not to keep the full output history when this multiapp has multiple entries");
  params.addParam<bool>("ignore_solve_not_converge",
                        false,
                        "True to continue main app even if a sub app's solve does not converge.");
  params.addParam<bool>("update_old_solution_when_keeping_solution_during_restore",
                        true,
                        "Whether to update the old solution vector (to the previous fixed point "
                        "iteration solution) when keeping the solution during restore.");
  return params;
}

FullSolveMultiApp::FullSolveMultiApp(const InputParameters & parameters)
  : MultiApp(parameters),
    _ignore_diverge(getParam<bool>("ignore_solve_not_converge")),
    _update_old_state_when_keeping_solution_during_restore(
        getParam<bool>("update_old_solution_when_keeping_solution_during_restore"))
{
  // You could end up with some dirty hidden behavior if you do this. We could remove this check,
  // but I don't think that it's sane to do so.
  if (_no_restore && (_app.isRecovering() || _app.isRestarting()))
    paramError("no_restore",
               "The parent app is restarting or recovering, restoration cannot be disabled");
  // Force the user to make a decision on updating or not the old state of variables
  if (_keep_solution_during_restore &&
      !isParamSetByUser("update_old_solution_when_keeping_solution_during_restore"))
    paramError("update_old_solution_when_keeping_solution_during_restore",
               "Due to 'keep_solution_during_restore' parameter being true, which is an "
               "optimization for fixed point iterations, the "
               "unrestored solution will be kept as the starting solution for the next solve "
               "of the MultiApp. You must set this parameter to decide if this solution should "
               "be copied as the old solution at the beginning of the next time step, "
               "or not. If the MultiApp is running a transient, you likely want to set this to "
               "true. If the MultiApp is a quasi-static simulation, you likely want to set "
               "this to false. If you don't know what this error message means, please set "
               "'keep_solution_during_restore' to false and no need to set "
               "'update_old_solution_when_keeping_solution_during_restore'.");
  if (isParamSetByUser("update_old_solution_when_keeping_solution_during_restore") &&
      !_keep_solution_during_restore)
    paramError("update_old_solution_when_keeping_solution_during_restore",
               "Should not be set if not keeping the solution during restore "
               "(keep_solution_during_restore=false)");
}

void
FullSolveMultiApp::restore(bool force)
{
  if (!_no_restore)
    MultiApp::restore(force);
}

void
FullSolveMultiApp::initialSetup()
{
  MultiApp::initialSetup();

  if (_has_an_app)
  {
    Moose::ScopedCommSwapper swapper(_my_comm);

    _executioners.resize(_my_num_apps);

    // Grab Executioner from each app
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      auto & app = _apps[i];
      Executioner * ex = app->getExecutioner();

      if (!ex)
        mooseError("Executioner does not exist!");

      if (_ignore_diverge)
      {
        TransientBase * tex = dynamic_cast<TransientBase *>(ex);
        if (tex && tex->parameters().get<bool>("error_on_dtmin"))
          mooseError("Requesting to ignore failed solutions, but 'Executioner/error_on_dtmin' is "
                     "true in sub-application. Set this parameter to false in sub-application to "
                     "avoid an error if Transient solve fails.");
      }

      ex->init();

      if (_update_old_state_when_keeping_solution_during_restore &&
          (appProblemBase(_first_local_app + i).getMaterialPropertyStorage().hasStatefulProperties()
#ifdef KOKKOS_ENABLED
           || appProblemBase(_first_local_app + i)
                  .getKokkosMaterialPropertyStorage()
                  .hasStatefulProperties()
#endif
               ))
        paramError(
            "update_old_solution_when_keeping_solution_during_restore",
            "While we are updating old solutions using the solution from the previous fixed "
            "point iteration, we are not updating the old stateful material properties as "
            "well. This is not consistent. We recommend you consider using the 'no_restore' "
            "parameter instead of 'keep_solution_during_restore', or stop using the latter.");
      if (_keep_solution_during_restore &&
          appProblemBase(_first_local_app + i)
              .hasSolutionState(2, Moose::SolutionIterationType::Time))
        mooseDoOnce(paramWarning(
            "keep_solution_during_restore",
            "This FullSolveMultiApp simulation(s) uses older time step variable states (notably "
            "from two time steps prior in transients). Due to 'keep_solution_during_restore' "
            "parameter being true, which is an optimization for fixed point iterations, the "
            "unrestored solution will be kept as the starting solution. It would normally be "
            "copied onto the 'old' state at the beginning of the first time step. This copy can be "
            "skipped using 'update_old_solution_when_keeping_solution_during_restore', while the "
            "copy of the old state onto the 'older' state and the stateful material properties "
            "state updates do not have such an option at this time. This warning relates to this "
            "inconsistency. If you suspect this is a problem, please set "
            "'keep_solution_during_restore' to false"));

      _executioners[i] = ex;
    }
  }
}

bool
FullSolveMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool auto_advance)
{
  if (!auto_advance)
    mooseError("FullSolveMultiApp is not compatible with auto_advance=false");

  if (!_has_an_app)
    return true;

  TIME_SECTION(_solve_step_timer);

  Moose::ScopedCommSwapper swapper(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  bool last_solve_converged = true;
  for (unsigned int i = 0; i < _my_num_apps; i++)
  {
    // reset output system if desired
    if (!getParam<bool>("keep_full_output_history"))
      _apps[i]->getOutputWarehouse().reset();
    // Prevent the copy of the post-FP iteration solution state onto the old vector
    if (!_update_old_state_when_keeping_solution_during_restore)
      appProblemBase(_first_local_app + i).skipNextForwardSolutionCopyToOld();

    Executioner * ex = _executioners[i];
    ex->execute();

    last_solve_converged = last_solve_converged && ex->lastSolveConverged();

    showStatusMessage(i);
  }

  return last_solve_converged || _ignore_diverge;
}

void
FullSolveMultiApp::showStatusMessage(unsigned int i) const
{
  if (!_fe_problem.verboseMultiApps() &&
      _apps[i]->getOutputWarehouse().getOutputs<Console>().size() > 0)
    return;
  else if (!_executioners[i]->lastSolveConverged())
    _console << COLOR_RED << "Subapp " << _apps[i]->name() << " solve Did NOT Converge!"
             << COLOR_DEFAULT << std::endl;
  else
    _console << COLOR_GREEN << "Subapp " << _apps[i]->name() << " solve converged!" << COLOR_DEFAULT
             << std::endl;
}
