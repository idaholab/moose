#include "OutputBlock.h"

#include "exodusII_io.h"
#include "mesh.h"

OutputBlock::OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<std::string>("file_base") = "out";
  _block_params.set<int>("interval") = 1;
  _block_params.set<bool>("exodus") = false;
  _block_params.set<bool>("gmv") = false;
  _block_params.set<bool>("tecplot") = false;
  _block_params.set<bool>("print_out_info") = false;
  _block_params.set<bool>("output_initial") = false;
}

void
OutputBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the OutputBlock Object\n";
#endif

  Moose::file_base = _block_params.get<std::string>("file_base");
  Moose::interval = _block_params.get<int>("interval");
  Moose::exodus_output = _block_params.get<bool>("exodus");
  Moose::gmv_output = _block_params.get<bool>("gmv");
  Moose::tecplot_output = _block_params.get<bool>("tecplot");
  Moose::print_out_info = _block_params.get<bool>("print_out_info");
  Moose::output_initial = _block_params.get<bool>("output_initial");

  visitChildren();
}

