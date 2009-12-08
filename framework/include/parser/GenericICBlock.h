#ifndef GENERICICBLOCK_H
#define GENERICICBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericICBlock;

template<>
InputParameters validParams<GenericICBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string>("variable", "", "The name of the variable to set the initial condition for.", true);
  return params;
}

class GenericICBlock: public ParserBlock
{
public:
  GenericICBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICICBLOCK_H
