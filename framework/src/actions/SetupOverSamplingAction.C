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

#include "SetupOverSamplingAction.h"

#include "Moose.h"
#include "Parser.h"
#include "Executioner.h"
#include "Output.h"
#include "FEProblem.h"
#include "OutputProblem.h"

#include "exodusII_io.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SetupOverSamplingAction>()
{
  // Note: I am specifically "skipping" the direct parent here because we want a subset of parameters
  // in this block for now.  We may make a common ancestor at some point to clean this up
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("file_base", "The desired oversampled solution output name without an extension.  If not supplied, the main file_base will be used with a '_oversample' suffix added.");

  params.addParam<unsigned int>("refinements", 1, "The number of refinements to output for the over sampled solution");

  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("nemesis", false, "Specifies that you would like Nemesis output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("tecplot_binary", false, "Specifies that you would like Tecplot binary output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");

  params.addParam<std::vector<std::string> >("output_variables", "A list of the variables that should be in the Exodus output file.  If this is not provided then all variables will be in the output.");

  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<Real>("iteration_plot_start_time", std::numeric_limits<Real>::max(), "Specifies a time after which the solution will be written following each nonlinear iteration");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");

  return params;
}


SetupOverSamplingAction::SetupOverSamplingAction(const std::string & name, InputParameters params) :
    SetupOutputAction(name, params)
{
}

void
SetupOverSamplingAction::act()
{
  Executioner * exec = Moose::executioner;
  FEProblem * problem = dynamic_cast<FEProblem *>(&exec->problem());
  if (!problem)
    mooseError("Can't get a handle to FEproblem");

  OutputProblem & out_problem = problem->getOutputProblem(getParam<unsigned int>("refinements"));

  Output & output = out_problem.out();  // can't use use this with coupled problems on different meshes

  if(!_pars.isParamValid("output_variables") && _parser_handle._problem != NULL)
  {
    FEProblem & fe_problem = *_parser_handle._problem;
    _pars.set<std::vector<std::string> >("output_variables") = fe_problem.getVariableNames();
  }

  // If no filebase was supplied in the parameters object - borrow the main problem's filebase
  if (!_pars.isParamValid("file_base"))
    _pars.set<std::string>("file_base") = problem->out().fileBase() + "_oversample";

  setupOutputObject(output, _pars);

  out_problem.outputInitial(getParam<bool>("output_initial"));

  if (_parser_handle._problem != NULL)
  {
    // TODO: handle this thru Problem interface
    FEProblem & fe_problem = *_parser_handle._problem;

#ifdef LIBMESH_ENABLE_AMR
    Adaptivity & adapt = fe_problem.adaptivity();
    if (adapt.isOn())
      output.sequence(true);
#endif //LIBMESH_ENABLE_AMR

    // TODO: Need to set these values on the OutputProblem
    output.interval(getParam<int>("interval"));
    output.iterationPlotStartTime(getParam<Real>("iteration_plot_start_time"));

  }

// Test to make sure that the user can write to the directory specified in file_base
  std::string base = "./" + getParam<std::string>("file_base");
  base = base.substr(0, base.find_last_of('/'));
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + getParam<std::string>("file_base"));
}
