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
#include "TransientBase.h"
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
  return params;
}

FullSolveMultiApp::FullSolveMultiApp(const InputParameters & parameters)
  : MultiApp(parameters), _ignore_diverge(getParam<bool>("ignore_solve_not_converge"))
{
  // You could end up with some dirty hidden behavior if you do this. We could remove this check,
  // but I don't think that it's sane to do so.
  if (_no_restore && (_app.isRecovering() || _app.isRestarting()))
    paramError("no_restore",
               "The parent app is restarting or recovering, restoration cannot be disabled");
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
