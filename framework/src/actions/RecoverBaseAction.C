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

  return params;
}


RecoverBaseAction::RecoverBaseAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
RecoverBaseAction::RecoverBaseObject(Output &output, InputParameters & params)
{
}

void
RecoverBaseAction::act()
{
  if(_app.isRecovering())
  {
    if(_app._recover_base.empty()) // If this is the case then we need to find the newest recovery file.
    {
      if (!_pars.isParamValid("file_base"))
        mooseError("\"Output/file_base\" must be valid if no recovery file is specified!");

      std::string newest_recovery_file = newestRestartFileWithBase(getParam<OutFileBase>("file_base") + "_restart");

      std::cout<<"\nUsing "<<newest_recovery_file<<" for recovery.\n"<<std::endl;

      _app._recover_base = newest_recovery_file;
    }
  }
}

std::string
RecoverBaseAction::newestRestartFileWithBase(std::string base_name)
{
  char * base_name_char = const_cast<char *>(base_name.c_str());

  std::string dir = dirname(base_name_char);
  std::string file_base = basename(base_name_char);

  pcrecpp::RE re_base_and_file_num("(.*?_restart_(\\d+)).*"); // Will pull out the full base and the file number simultaneously

  tinydir_dir tdir;

  if(tinydir_open(&tdir, dir.c_str()) == -1)
    mooseError("Cannot open directory!");

  unsigned int max_file_num = 0;
  std::string max_base;

  while(tdir.has_next)
  {
    tinydir_file file;

    if(tinydir_readfile(&tdir, &file) == -1)
      mooseError("Error getting file!");

    std::string file_name = file.name;

    if((!file.is_dir) && file_name.find(file_base) == 0) // Does the file start with the base?
    {
      std::string the_base;
      int file_num = 0;
      re_base_and_file_num.FullMatch(file_name, &the_base, &file_num);

      if(file_num > max_file_num)
        max_base = the_base;
    }

    tinydir_next(&tdir);
  }

  return dir + "/" + max_base;
}
