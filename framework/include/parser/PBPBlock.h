#ifndef PBPBLOCK_H
#define PBPBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class PBPBlock;
template<>
InputParameters validParams<PBPBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::vector<std::string> >("solve_order", "TODO: docstring", true);
  params.addParam<std::vector<std::string> >("preconditioner", "TODO: docstring", true);
  params.addParam<std::vector<std::string> >("off_diag_row", "TODO: docstring", false);
  params.addParam<std::vector<std::string> >("off_diag_column", "TODO: docstring", false);
  return params;
}

class PBPBlock: public ParserBlock
{
public:
  PBPBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};


  

#endif //PBPBLOCK_H
