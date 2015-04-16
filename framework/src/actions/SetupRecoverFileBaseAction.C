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
#include "SetupRecoverFileBaseAction.h"
#include "MooseApp.h"
#include "OutputWarehouse.h"
#include "Checkpoint.h"
#include "MooseObjectAction.h"

// External includes
#include "pcrecpp.h"
#include "tinydir.h"

template<>
InputParameters validParams<SetupRecoverFileBaseAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupRecoverFileBaseAction::SetupRecoverFileBaseAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

SetupRecoverFileBaseAction::~SetupRecoverFileBaseAction()
{
}

void
SetupRecoverFileBaseAction::act()
{
  // Do nothing if the App is not recovering
  if (!_app.isRecovering())
    return;

  // Build the list of all possible checkpoint files for recover
  std::set<std::string> checkpoint_files;
  getCheckpointFiles(checkpoint_files);

  // Get the most current file, if it hasn't been set directly
  if (!_app.hasRecoverFileBase())
  {
    std::string recovery_file_base = getRecoveryFileBase(checkpoint_files);
    _app.setRecoverFileBase(recovery_file_base);
  }

  // Set the recover file base in the App
  _console << "\nUsing " << _app.getRecoverFileBase() << " for recovery.\n" << std::endl;
}

std::string
SetupRecoverFileBaseAction::getRecoveryFileBase(const std::set<std::string> checkpoint_files)
{
  // Create storage for newest restart files
  // Note that these might have the same modification time if the simulation was fast.
  // In that case we're going to save all of the "newest" files and sort it out momentarily
  time_t newest_time = 0;
  std::vector<std::string> newest_restart_files;

  // Loop through all possible files and store the newest
  for (std::set<std::string>::iterator it = checkpoint_files.begin(); it != checkpoint_files.end(); ++it)
  {
      struct stat stats;
      stat(it->c_str(), &stats);

      time_t mod_time = stats.st_mtime;
      if (mod_time > newest_time)
      {
        newest_restart_files.clear(); // If the modification time is greater, clear the list
        newest_time = mod_time;
      }

      if (mod_time == newest_time)
        newest_restart_files.push_back(*it);
  }

  // Loop through all of the newest files according the number in the file name
  int max_file_num = -1;
  std::string max_base;
  pcrecpp::RE re_base_and_file_num("(.*?(\\d+))\\..*"); // Will pull out the full base and the file number simultaneously

  // Now, out of the newest files find the one with the largest number in it
  for (unsigned int i=0; i<newest_restart_files.size(); i++)
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

  // Error if nothing was located
  if (max_file_num == -1)
    mooseError("Unable to find suitable recovery file!");

  return max_base;
}

void
SetupRecoverFileBaseAction::getCheckpointFiles(std::set<std::string> & files)
{
  // Extract the CommonOutputAction
  const Action* common = _awh.getActionsByName("common_output")[0];

  // Storage for the directory names
  std::set<std::string> checkpoint_dirs;

  // If file_base is set in CommonOutputAction, add this file to the list of potential checkpoint files
  if (common->isParamValid("file_base"))
    checkpoint_dirs.insert(common->getParam<std::string>("file_base") + "_cp");
  // Case for normal application or master in a Multiapp setting
  else if (_app.getOutputFileBase().empty())
    checkpoint_dirs.insert(FileOutput::getOutputFileBase(_app, "_out_cp"));
  // Case for a sub app in a Multiapp setting
  else
    checkpoint_dirs.insert(_app.getOutputFileBase() + "_cp");

  // Add the directories from any existing checkpoint objects
  const std::vector<Action *> actions = _awh.getActionsByName("add_output");
  for (std::vector<Action *>::const_iterator it = actions.begin(); it != actions.end(); ++it)
  {
    // Get the parameters from the MooseObjectAction
    MooseObjectAction * moose_object_action = static_cast<MooseObjectAction *>(*it);
    const InputParameters & params = moose_object_action->getObjectParams();

    // Loop through the actions and add the necessary directories to the list to check
    if (moose_object_action->getParam<std::string>("type") == "Checkpoint")
    {
      if (params.isParamValid("file_base"))
        checkpoint_dirs.insert(common->getParam<std::string>("file_base") + "_cp");
      else
      {
        std::ostringstream oss;
        oss << "_" << (*it)->getShortName() << "_cp";
        checkpoint_dirs.insert(FileOutput::getOutputFileBase(_app, oss.str()));
      }
    }
  }

  // Loop through the possible directories and extract the files
  for (std::set<std::string>::const_iterator it = checkpoint_dirs.begin(); it != checkpoint_dirs.end(); ++it)
  {
    tinydir_dir dir;
    dir.has_next = 0; // Avoid a garbage value in has_next (clang StaticAnalysis)
    tinydir_open(&dir, it->c_str());

    while (dir.has_next)
    {
      tinydir_file file;
      file.is_dir = 0; // Avoid a garbage value in is_dir (clang StaticAnalysis)
      tinydir_readfile(&dir, &file);

      if (!file.is_dir)
        files.insert(*it + "/" + file.name);

      tinydir_next(&dir);
    }

    tinydir_close(&dir);
  }
}
