#include "OutputBlock.h"

#include "exodusII_io.h"
#include "mesh.h"

OutputBlock::OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  addParam<std::string>("file_base", "out", "The desired solution output name without an extension", true);
  addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file", false);
  addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)", false);
  addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)", false);
  addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)", false);
  addParam<bool>("print_out_info", false, "Specifies that you would like to see more verbose output information on STDOUT", false);
  addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file", false);
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

