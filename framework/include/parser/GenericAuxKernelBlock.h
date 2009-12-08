#ifndef GENERICAUXKERNELBLOCK_H
#define GENERICAUXKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericAuxKernelBlock;

template<>
InputParameters validParams<GenericAuxKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string>("variable", "", "The Aux Kernel Name used in your model", true);
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Kernel is coupled to.", false);
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Kernel which correspond with the coupled_as names", false);
  return params;
}

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H
