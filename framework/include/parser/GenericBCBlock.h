#ifndef GENERICBCBLOCK_H
#define GENERICBCBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericBCBlock;

template<>
InputParameters validParams<GenericBCBlock>();

class GenericBCBlock: public ParserBlock
{
public:
  GenericBCBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICBCBLOCK_H
