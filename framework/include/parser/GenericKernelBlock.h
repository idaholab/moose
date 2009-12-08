#ifndef GENERICKERNELBLOCK_H
#define GENERICKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericKernelBlock;

template<>
InputParameters validParams<GenericKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Kernel will act on.");

  params.addParam<int>("block", "The mesh file block for which this kernel is active.  If not set then this Kernel will be active everywhere.");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Kernel is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Kernel which correspond with the coupled_as names");
  return params;
}

class GenericKernelBlock: public ParserBlock
{
public:
  GenericKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICKERNELBLOCK_H
