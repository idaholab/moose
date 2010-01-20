#ifndef VARIABLESBLOCK_H
#define VARIABLESBLOCK_H

#include "ParserBlock.h"

//libmesh Includes
#include "dof_map.h"

class VariablesBlock;

template<>
InputParameters validParams<VariablesBlock>();

class VariablesBlock: public ParserBlock
{
public:
  VariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
  virtual void copyNodalValues(const std::string &system_name);
};

#endif //VARIABLESBLOCK_H
