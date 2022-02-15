//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FullSolveMultiApp.h"
#include "LayeredSideDiffusiveFluxAverage.h"
#include "Executioner.h"
#include "Transient.h"
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
      "no_backup_and_restore",
      false,
      "True to turn off backup/restore for this multiapp. This is useful when doing steady-state "
      "Picard iterations where we want to use the solution of previous Picard iteration as the "
      "initial guess of the current Picard iteration");
  params.addParam<bool>("reinit_after_solve",
                        false,
                        "Switch used to reint the multiapp to the initial conditions after every "
                        "fixed-point solve. This is used to enable the repeated solution of the "
                        "same problem with changing conditions (like parameters).");
  params.addParam<bool>(
      "keep_full_output_history",
      false,
      "Whether or not to keep the full output history when this multiapp has multiple entries");
  params.addParam<bool>("ignore_solve_not_converge",
                        false,
                        "True to continue main app even if a sub app's solve does not converge.");
  return params;
}

FullSolveMultiApp::FullSolveMultiApp(const InputParameters & parameters)
  : MultiApp(parameters), _ignore_diverge(getParam<bool>("ignore_solve_not_converge"))
{
}

void
FullSolveMultiApp::backup()
{
  if (getParam<bool>("no_backup_and_restore"))
    return;
  else
    MultiApp::backup();
}

void
FullSolveMultiApp::restore(bool /*force*/)
{
  if (getParam<bool>("no_backup_and_restore"))
    return;
  else
    MultiApp::restore();
}

void
FullSolveMultiApp::reinitialize()
{
  // Similarly to restore, which is used for the fixed-point iteration,
  // we reinitialize our problem if it is requested by the user.
  if (getParam<bool>("reinit_after_solve") && _has_an_app)
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      resetApp(i);
      initialSetup();
    }
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
        Transient * tex = dynamic_cast<Transient *>(ex);
        if (tex && tex->parameters().get<bool>("error_on_dtmin"))
          mooseError("Requesting to ignore failed solutions, but 'Executioner/error_on_dtmin' is "
                     "true in sub-application. Set this parameter to false in sub-application to "
                     "avoid an error if Transient solve fails.");
      }

      ex->init();

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

    bool show = (_fe_problem.verboseMultiApps() ||
                 _apps[i]->getOutputWarehouse().getOutputs<Console>().size() == 0);

    Executioner * ex = _executioners[i];
    ex->execute();
    if (!ex->lastSolveConverged())
    {
      last_solve_converged = false;
      if (show)
        _console << COLOR_RED << "Subapp " << _apps[i]->name() << " solve Did NOT Converge!"
                 << COLOR_DEFAULT << std::endl;
    }
    else if (show)
      _console << COLOR_GREEN << "Subapp " << _apps[i]->name() << " solve converged!"
               << COLOR_DEFAULT << std::endl;
  }

  return last_solve_converged || _ignore_diverge;
}
