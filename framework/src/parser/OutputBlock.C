#include "OutputBlock.h"
#include "Moose.h"

#include "exodusII_io.h"
#include "mesh.h"

template<>
InputParameters validParams<OutputBlock>()
{
  InputParameters params = validParams<ParserBlock>();

  params.addParam<std::string>("file_base", "out", "The desired solution output name without an extension");
  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("print_out_info", false, "Specifies that you would like to see more verbose output information on STDOUT");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  return params;
}

OutputBlock::OutputBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register Output prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
  addPrereq("BCs");
  addPrereq("AuxBCs");
  addPrereq("Materials");
  addPrereq("Executioner");
}

void
OutputBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the OutputBlock Object\n";
#endif

  Moose::file_base = getParamValue<std::string>("file_base");
  Moose::interval = getParamValue<int>("interval");
  Moose::exodus_output = getParamValue<bool>("exodus");
  Moose::gmv_output = getParamValue<bool>("gmv");
  Moose::tecplot_output = getParamValue<bool>("tecplot");
  Moose::print_out_info = getParamValue<bool>("print_out_info");
  Moose::output_initial = getParamValue<bool>("output_initial");

  visitChildren();
}

