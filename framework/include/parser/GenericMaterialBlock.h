#ifndef GENERICMATERIALBLOCK_H
#define GENERICMATERIALBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericMaterialBlock;

template<>
InputParameters validParams<GenericMaterialBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::vector<int> >("block", "The list of blocks for which this material is active on");
  
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this object is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this object which correspond with the coupled_as names");
  return params;
}

class GenericMaterialBlock: public ParserBlock
{
public:
  GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICMATERIALBLOCK_H
