#include "OutputBlock.h"
#include "Moose.h"
#include "Parser.h"
#include "Executioner.h"
#include "Output.h"
#include "MProblem.h"

#include "exodusII_io.h"
#include "mesh.h"

#ifdef LIBMESH_HAVE_PETSC
//#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC


template<>
InputParameters validParams<OutputBlock>()
{
  InputParameters params = validParams<ParserBlock>();

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

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<bool>("print_linear_residuals", false, "Specifies whether the linear residuals are printed during the solve");
#endif

  return params;
}


OutputBlock::OutputBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  addPrereq("Executioner");
#if 0
  // Register Output prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
//  addPrereq("AuxVariables");
  addPrereq("Kernels");
//  addPrereq("AuxKernels");
  addPrereq("BCs");
//  addPrereq("AuxBCs");
  addPrereq("Materials");
  addPrereq("Executioner");
#endif
}

void
OutputBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the OutputBlock Object\n";
#endif

  // FIXME: uncomm
  Executioner * exec = _parser_handle._executioner;
  Moose::Problem & problem = exec->problem();
  Moose::Output & output = problem.out();                       // can't use use this with coupled problems on different meshes

  //  Moose::Output & output = _parser_handle._problem->out();
  output.fileBase(getParamValue<std::string>("file_base"));
  if (getParamValue<bool>("exodus"))
    output.addExodus();

//  _parser_handle._executioner->outputInitial(getParamValue<bool>("output_initial"));
  exec->outputInitial(getParamValue<bool>("output_initial"));

#if 0
  _moose_system._interval = getParamValue<int>("interval");
  _moose_system._gmv_output = getParamValue<bool>("gmv");
  _moose_system._tecplot_output = getParamValue<bool>("tecplot");
  _moose_system._tecplot_binary_output = getParamValue<bool>("tecplot_binary");
  _moose_system._xda_output = getParamValue<bool>("xda");
  _moose_system._postprocessor_screen_output = getParamValue<bool>("postprocessor_screen");
  _moose_system._postprocessor_csv_output = getParamValue<bool>("postprocessor_csv");
  _moose_system._postprocessor_ensight_output = getParamValue<bool>("postprocessor_ensight");
  _moose_system._postprocessor_gnuplot_output = getParamValue<bool>("postprocessor_gnuplot");
  _moose_system._gnuplot_format = getParamValue<std::string>("gnuplot_format");
  _moose_system._print_out_info = getParamValue<bool>("print_out_info");

#ifdef LIBMESH_HAVE_PETSC
  if (getParamValue<bool>("print_linear_residuals"))
    PetscOptionsSetValue("-ksp_monitor", PETSC_NULL);
#endif
#endif

  visitChildren();
}

