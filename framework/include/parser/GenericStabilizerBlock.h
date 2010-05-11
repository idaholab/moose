#ifndef GENERICSTABILIZERBLOCK_H
#define GENERICSTABILIZERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericStabilizerBlock;

template<>
InputParameters validParams<GenericStabilizerBlock>();

class GenericStabilizerBlock: public ParserBlock
{
public:
  GenericStabilizerBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICSTABILIZERBLOCK_H
