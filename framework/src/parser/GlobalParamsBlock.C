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

#include "GlobalParamsBlock.h"

template<>
InputParameters validParams<GlobalParamsBlock>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  /* GlobalParams should not have children or other standard public block attributes */
  params.addPrivateParam<std::vector<std::string> >("active", blocks);
  params.addPrivateParam<ParserBlock *>("parent");
  params.addPrivateParam<Parser *>("parser_handle");
  params.addPrivateParam<std::string>("type", "GlobalParams");
  return params;
}

GlobalParamsBlock::GlobalParamsBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{}

void
GlobalParamsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GlobalParamsBlock Object\n";
#endif

  visitChildren();
}  

  
