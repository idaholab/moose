#include "OutputBlock.h"
#include "Moose.h"

#include "exodusII_io.h"
#include "mesh.h"

OutputBlock::OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{
  // Register the execution Prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
  addPrereq("BCs");
  addPrereq("AuxBCs");
  addPrereq("Materials");
  addPrereq("Execution");
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

