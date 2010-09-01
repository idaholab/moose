#include "GlobalParamsBlock.h"

template<>
InputParameters validParams<GlobalParamsBlock>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  /* GlobalParams should not have children or other standard block attributes */
  params.addPrivateParam<std::vector<std::string> >("active", blocks);
  params.addPrivateParam<ParserBlock *>("parent");
  params.addPrivateParam<Parser *>("parser_handle");
  return params;
}

GlobalParamsBlock::GlobalParamsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
GlobalParamsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GlobalParamsBlock Object\n";
#endif

  visitChildren();
}  

  
