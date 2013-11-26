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

#include "SetupOutputNameAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "OutputProblem.h"
#include "unistd.h"

template<>
InputParameters validParams<SetupOutputNameAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<OutFileBase>("file_base", "out", "The desired solution output name without an extension (Defaults to the mesh file name + '_out' or 'out' if generating the mesh by some other means)");
  return params;
}

SetupOutputNameAction::SetupOutputNameAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

SetupOutputNameAction::~SetupOutputNameAction()
{
}

void
SetupOutputNameAction::act()
{
  OutFileBase file_base = _pars.get<OutFileBase>("file_base");

  // Has the filebase been overridden at the application level?
  if (_app.getOutputFileBase() != "")
    file_base = _app.getOutputFileBase();

  if (_problem != NULL)
  {
    Output & output = _problem->out();                       // can't use use this with coupled problems on different meshes

    if(_pars.isParamValid("output_if_base_contains"))
    {
      const std::vector<std::string> & strings = _pars.get<std::vector<std::string> >("output_if_base_contains");

      bool found_it = false;
      for(unsigned int i=0; i<strings.size(); i++)
        found_it = found_it || (file_base.find(strings[i]) != std::string::npos);

      if(!found_it) // Didn't find a match so no output should be done
        return;
    }

    output.fileBase(file_base);

    // Test to make sure that the user can write to the directory specified in file_base
    std::string base = "./" + file_base;
    base = base.substr(0, base.find_last_of('/'));
    // TODO: We have a function that tests read/write in the Parser namespace.  We should probably
    // use that instead of creating another one here
    if (access(base.c_str(), W_OK) == -1)
      mooseError("Can not write to directory: " + base + " for file base: " + file_base);
  }
}
