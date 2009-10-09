#include "OutputBlock.h"

#include "exodusII_io.h"
#include "mesh.h"

OutputBlock::OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<std::string>("file_base") = "out";
}

void
OutputBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the OutputBlock Object\n";
#endif

  ExodusII_IO(*Moose::mesh).write_equation_systems(_block_params.get<std::string>("file_base"),
                                                   *Moose::equation_system);
  
}
