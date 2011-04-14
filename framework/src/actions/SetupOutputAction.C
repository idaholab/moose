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

#include "SetupOutputAction.h"

#include "Moose.h"
#include "Parser.h"
#include "Executioner.h"
#include "Output.h"
#include "MProblem.h"

#include "exodusII_io.h"
#include "MooseMesh.h"


template<>
InputParameters validParams<SetupOutputAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("file_base", "out", "The desired solution output name without an extension");
  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("tecplot_binary", false, "Specifies that you would like Tecplot binary output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");
  params.addParam<bool>("postprocessor_screen", true, "Specifies that you would like PostProcessor output to the screen (stdout)"); 
  params.addParam<bool>("postprocessor_csv", false, "Specifies that you would like a PostProcessor comma seperated values file"); 
  params.addParam<bool>("postprocessor_ensight", false, "Specifies that you would like a PostProcessor ensight output file"); 
  params.addParam<bool>("postprocessor_gnuplot", false, "Specifies that you would like plots of the postprocessor output"); 
  params.addParam<std::string>("gnuplot_format", "ps", "Specifies which output format gnuplot will produce. Currently supported: ps, gif, and png"); 
  params.addParam<bool>("print_out_info", false, "Specifies that you would like to see more verbose output information on STDOUT");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  params.addParam<bool>("output_displaced", false, "Requests that displaced mesh files are written at each solve");

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<bool>("print_linear_residuals", false, "Specifies whether the linear residuals are printed during the solve");
#endif
  
  
  return params;
}


SetupOutputAction::SetupOutputAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupOutputAction::act()
{
  Executioner * exec = _parser_handle._executioner;
  Problem & problem = exec->problem();
  Output & output = problem.out();                       // can't use use this with coupled problems on different meshes

  output.fileBase(getParam<std::string>("file_base"));

  if (getParam<bool>("exodus")) output.add(Output::EXODUS);
  if (getParam<bool>("gmv")) output.add(Output::GMV);
  if (getParam<bool>("tecplot")) output.add(Output::TECPLOT);
  if (getParam<bool>("tecplot_binary")) output.add(Output::TECPLOT_BIN);
  if (getParam<bool>("xda")) output.add(Output::XDA);

#if 0
    _moose_system._print_out_info = getParam<bool>("print_out_info");
#endif

  exec->outputInitial(getParam<bool>("output_initial"));
  problem.outputInitial(getParam<bool>("output_initial"));

  if (_parser_handle._problem != NULL)
  {
    // TODO: handle this thru Problem interface
    MProblem & mproblem = *_parser_handle._problem;
    mproblem._postprocessor_screen_output = getParam<bool>("postprocessor_screen");
    mproblem._postprocessor_csv_output = getParam<bool>("postprocessor_csv");
    mproblem._postprocessor_ensight_output = getParam<bool>("postprocessor_ensight");
    mproblem._postprocessor_gnuplot_output = getParam<bool>("postprocessor_gnuplot");
    mproblem._gnuplot_format = getParam<std::string>("gnuplot_format");

    mproblem.outputDisplaced(getParam<bool>("output_displaced"));

#ifdef LIBMESH_ENABLE_AMR
    Adaptivity & adapt = mproblem.adaptivity();
    if (adapt.isOn())
      output.sequence(true);
#endif //LIBMESH_ENABLE_AMR

    output.interval(getParam<int>("interval"));
  }

#ifdef LIBMESH_HAVE_PETSC
//  if (getParam<bool>("print_linear_residuals"))
//    PetscOptionsSetValue("-ksp_monitor", PETSC_NULL);
#endif

 // Test to make sure that the user can write to the directory specified in file_base
  std::string base = "./" + getParam<std::string>("file_base");
  base = base.substr(0, base.find_last_of('/'));
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + getParam<std::string>("file_base"));
 
}
