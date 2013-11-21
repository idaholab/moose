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

#include "RecoverBaseAction.h"

#include "MooseApp.h"
#include "MooseUtils.h"

#include "tinydir.h"
#include "pcrecpp.h"

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

template<>
InputParameters validParams<RecoverBaseAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<OutFileBase>("file_base", "The desired solution output name without an extension (Defaults to the mesh file name + '_out' or 'out' if generating the mesh by some other means)");
  params.addParam<std::string>("checkpoint_dir_suffix", "cp", "This will be appended to the file_base to create the directory name for checkpoint files.");

  params.addParamNamesToGroup("checkpoint_dir_suffix", "Checkpoint");
  return params;
}


RecoverBaseAction::RecoverBaseAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
RecoverBaseAction::RecoverBaseObject(Output & /*output*/, InputParameters & /*params*/)
{
}

void
RecoverBaseAction::act()
{
  if(_app.isRecovering())
  {
    // If this is the case then we need to find the newest recovery file.
    if(_app.getRecoverFileBase().empty())
    {
      if (!_pars.isParamValid("file_base"))
        mooseError("\"Output/file_base\" must be valid if no recovery file is specified!");

      std::string newest_recovery_file = newestRestartFileWithBase(getParam<OutFileBase>("file_base"));

      Moose::out << "\nUsing " << newest_recovery_file << " for recovery.\n" << std::endl;

      _app.setRecoverFileBase(newest_recovery_file);
    }
  }
}

std::string
RecoverBaseAction::newestRestartFileWithBase(std::string base_name)
{
  char * base_name_char = const_cast<char *>(base_name.c_str());

  std::string dir = dirname(base_name_char);
  std::string file_base = basename(base_name_char);

  dir = dir + "/" + file_base + "_" + getParam<std::string>("checkpoint_dir_suffix");

  pcrecpp::RE re_base_and_file_num("(.*?(\\d+))\\..*"); // Will pull out the full base and the file number simultaneously

  tinydir_dir tdir;

  if(tinydir_open(&tdir, dir.c_str()) == -1)
    mooseError("Cannot open directory!");

  time_t newest_time = 0;
  std::vector<std::string> newest_restart_files;

  // First, the newest candidate files.
  // Note that these might have the same modification time if the simulation was fast
  // In that case we're going to save all of the "newest" files and sort it out momentarily
  while(tdir.has_next)
  {
    tinydir_file file;

    if(tinydir_readfile(&tdir, &file) == -1)
    {
      tinydir_next(&tdir);
      continue;
    }

    std::string file_name = file.name;

    if((!file.is_dir))
    {
      struct stat stats;

      std::string full_path = dir + "/" + file_name;

      stat(full_path.c_str(), &stats);

      time_t mod_time = stats.st_mtime;

      if(mod_time > newest_time)
      {
        newest_restart_files.clear();
        newest_time = mod_time;
      }

      if(mod_time == newest_time)
        newest_restart_files.push_back(file_name);
    }

    tinydir_next(&tdir);
  }

  int max_file_num = 0;
  std::string max_base;

  // Now, out of the newest files find the one with the largest number in it
  for(unsigned int i=0; i<newest_restart_files.size(); i++)
  {
    std::string file_name = newest_restart_files[i];

    std::string the_base;
    int file_num = 0;

    re_base_and_file_num.FullMatch(file_name, &the_base, &file_num);

    if(file_num > max_file_num)
      max_base = the_base;
  }

  return dir + "/" + max_base;
}
