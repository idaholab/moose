#ifndef GENERICBCBLOCK_H
#define GENERICBCBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericBCBlock;

template<>
InputParameters validParams<GenericBCBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string>("variable", "", "The BC Name used in your model", true);
  params.addParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary", true);
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this object is coupled to.", false);
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this object which correspond with the coupled_as names", false);
  return params;
}

class GenericBCBlock: public ParserBlock
{
public:
  GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICBCBLOCK_H
