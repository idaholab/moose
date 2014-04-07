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
#include "CommonOutputAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "Exodus.h"
#include "OutputWarehouse.h"
#include "FileOutputter.h"

// Extrnal includes
#include "tinydir.h"
#include "pcrecpp.h"
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

template<>
InputParameters validParams<CommonOutputAction>()
{
   InputParameters params = validParams<Action>();

   // Short-cut methods for typical output objects
   params.addParam<bool>("exodus", false, "Output the results using the default settings for Exodus output");
   params.addParam<bool>("nemesis", false, "Output the results using the default settings for Nemesis output");
   params.addParam<bool>("console", false, "Output the results using the default settings for Console output");
   params.addParam<bool>("csv", false, "Output the scalar variable and postprocessors to a *.csv file using the default CSV output.");
   params.addParam<bool>("vtk", false, "Output the results using the default settings for VTK output");
   params.addParam<bool>("xda", false, "Output the results using the default settings for XDA/XDR output (ascii)");
   params.addParam<bool>("xdr", false, "Output the results using the default settings for XDA/XDR output (binary)");
   params.addParam<bool>("checkpoint", false, "Create checkpoint files using the default options.");
   params.addParam<bool>("gmv", false, "Output the results using the default settings for GMV output");
   params.addParam<bool>("tecplot", false, "Output the results using the default settings for Tecplot output");
   params.addParam<bool>("gnuplot", false, "Output the scalar and postprocessor results using the default settings for GNUPlot output");
   params.addParam<bool>("solution_history", false, "Print a solution history file (.slh) using the default settings");

   // Common parameters
   params.addParam<bool>("output_initial", false,  "Request that the initial condition is output to the solution file");
   params.addParam<bool>("output_intermediate", true, "Request that all intermediate steps (not initial or final) are output");
   params.addParam<bool>("output_final", false, "Force the final timestep to be output, regardless of output interval");
   params.addParam<std::string>("file_base", "Common file base name to be utilized with all output objects");
   params.addParam<std::vector<std::string> >("output_if_base_contains", "If this is supplied then output will only be done in the case that the output base contains one of these strings.  This is helpful in outputting only a subset of outputs when using MultiApps.");
   params.addParam<unsigned int>("interval", 1, "The interval at which timesteps are output to the solution file");
   params.addParam<std::vector<Real> >("sync_times", "Times at which the output and solution is forced to occur");
   params.addParam<std::vector<VariableName> >("hide", "A list of the variables and postprocessors that should NOT be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");
   params.addParam<std::vector<VariableName> >("show", "A list of the variables and postprocessors that should be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

   // Add super-secret parameeters for creating running --recover via the test harness
   params.addPrivateParam<bool>("auto_recovery_part1", false);
   params.addPrivateParam<bool>("auto_recovery_part2", false);

   // Return object
   return params;
}

CommonOutputAction::CommonOutputAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _action_params(_action_factory.getValidParams("AddOutputAction"))
{
  // Set the ActionWarehouse pointer in the parameters that will be passed to the actions created with this action
  _action_params.set<ActionWarehouse *>("awh") = &_awh;
}

void
CommonOutputAction::act()
{
  // Store the common output parameters in the OutputWarehouse
  _app.getOutputWarehouse().setCommonParameters(&_pars);

  // Create the actions for the short-cut methods
  if (getParam<bool>("exodus"))
    create("Exodus");

  if (getParam<bool>("nemesis"))
    create("Nemesis");

  if (getParam<bool>("console"))
    create("Console");

  if (getParam<bool>("csv"))
    create("CSV");

  if (getParam<bool>("vtk"))
    create("VTK");

  if (getParam<bool>("xda"))
    create("XDA");

  if (getParam<bool>("xdr"))
    create("XDR");

  if (getParam<bool>("checkpoint"))
    create("Checkpoint");

  if (getParam<bool>("gmv"))
    create("GMV");

  if (getParam<bool>("tecplot"))
    create("Tecplot");

  if (getParam<bool>("gnuplot"))
    create("GNUPlot");

  if (getParam<bool>("solution_history"))
    create("SolutionHistory");

  // This creates a checkpoint block used by the test harness for testing recovery (i.e., ./run_tests --recover)
  if (getParam<bool>("auto_recovery_part1"))
    createAutoRecoveryCheckpointObject();

  if (_app.isRecovering() && !_app.hasRecoverFileBase())
    setRecoverFileBase();
}

void
CommonOutputAction::create(std::string object_type)
{
  // Set the 'type =' parameters for the desired object
  _action_params.set<std::string>("type") = object_type;

  // Create the complete object name (use lower case for the object_name)
  std::transform(object_type.begin(), object_type.end(), object_type.begin(), ::tolower);
  std::string long_name("Outputs/");
  long_name += object_type;

  // Create the action
  MooseObjectAction * action = static_cast<MooseObjectAction *>(_action_factory.create("AddOutputAction", long_name, _action_params));

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}

void
CommonOutputAction::createAutoRecoveryCheckpointObject()
{
  // Set the 'type =' parameters for the desired object
  _action_params.set<std::string>("type") = "Checkpoint";

  // Create the complete object name (use lower case for the object_name)
  std::string long_name("Outputs/auto_recovery_checkpoint");

  // Create the action
  MooseObjectAction * action = static_cast<MooseObjectAction *>(_action_factory.create("AddOutputAction", long_name, _action_params));

  // Set the suffix for the auto recovery checkpoint output so when recovering the location is known
  action->getObjectParams().set<std::string>("suffix") = "auto_recovery";

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}

void
CommonOutputAction::setRecoverFileBase()
{
  // Extract the default directory for recover files
  std::string dir = getRecoveryDirectory();

  pcrecpp::RE re_base_and_file_num("(.*?(\\d+))\\..*"); // Will pull out the full base and the file number simultaneously

  tinydir_dir tdir;

  if (tinydir_open(&tdir, dir.c_str()) == -1)
    mooseError("Cannot open directory: " << dir);

  time_t newest_time = 0;
  std::vector<std::string> newest_restart_files;

  // First, the newest candidate files.
  // Note that these might have the same modification time if the simulation was fast
  // In that case we're going to save all of the "newest" files and sort it out momentarily
  while(tdir.has_next)
  {
    tinydir_file file;

    if (tinydir_readfile(&tdir, &file) == -1)
    {
      tinydir_next(&tdir);
      continue;
    }

    std::string file_name = file.name;

    if ((!file.is_dir))
    {
      struct stat stats;

      std::string full_path = dir + "/" + file_name;

      stat(full_path.c_str(), &stats);

      time_t mod_time = stats.st_mtime;
      if (mod_time > newest_time)
      {
        newest_restart_files.clear();
        newest_time = mod_time;
      }

      if (mod_time == newest_time)
        newest_restart_files.push_back(file_name);
    }

    tinydir_next(&tdir);
  }

  int max_file_num = -1;
  std::string max_base;

  // Now, out of the newest files find the one with the largest number in it
  for(unsigned int i=0; i<newest_restart_files.size(); i++)
  {
    std::string file_name = newest_restart_files[i];

    std::string the_base;
    int file_num = 0;

    re_base_and_file_num.FullMatch(file_name, &the_base, &file_num);

    if (file_num > max_file_num)
    {
      max_file_num = file_num;
      max_base = the_base;
    }
  }

  if (max_file_num == -1)
    mooseError("Unable to find suitable recovery file!");

  std::string recovery = dir + "/" + max_base;

  Moose::out << "\nUsing " << recovery << " for recovery.\n" << std::endl;

  _app.setRecoverFileBase(recovery);
}

std::string
CommonOutputAction::getRecoveryDirectory()
{
  // The TestHarness uses a specific checkpoint outputter and naming
  if (getParam<bool>("auto_recovery_part2"))
  {
    std::string file_base;
    if (isParamValid("file_base"))
      file_base = _pars.get<std::string>("file_base");
    if (file_base.empty())
      file_base = FileOutputter::getOutputFileBase(_app);

    // Build and return the complete name for the recovery directory
    return file_base + "_auto_recovery";
  }

  // The string to be returned
  std::string full_name;

  // Get a list of actions
  const std::vector<Action *> & actions = _awh.getActionsByName("add_output");

  // Counter for producing a warning if multiple checkpoint objects found
  unsigned int cnt = 0;

  // Locate Checkpoint objects
  for (std::vector<Action *>::const_iterator it = actions.begin(); it != actions.end(); ++it)
  {
    // The name and type of object being created by the action
    std::string type = (*it)->parameters().get<std::string>("type");
    std::string name = (*it)->name();

    // The recovery file base is derived from the checkpoint output.
    /* Note, if multiple checkpoint outputter are given the recover system simply uses
     * the last checkpoint object in the actions as the guess for file recovery; however,
     * a warning is produced for the user */
    if (type.compare("Checkpoint") == 0)
    {
      // Extract the object parameters
      MooseObjectAction * cp = dynamic_cast<MooseObjectAction *>(*it);
      InputParameters & obj_pars = cp->getObjectParams();

      // Get the file base, checking the common and local parameters before using the default.
      std::string file_base;
      if (isParamValid("file_base"))
        file_base = _pars.get<std::string>("file_base");

      if (obj_pars.isParamValid("file_base"))
        file_base = obj_pars.get<std::string>("file_base");

      if (file_base.empty())
        file_base = FileOutputter::getOutputFileBase(_app);

      // Build the complete name for the recovery directory
      full_name = file_base + "_" + cp->getObjectParams().get<std::string>("suffix");

      // Increment counter
      cnt++;
    }
  }

  // Produce warning when multiple checkpoint outputters exist
  if (cnt > 1)
    mooseWarning("Multiple Checkpoint outputters detected, consider providing a file base to the --recover option.");

  // Return the directory
  return full_name;
}
