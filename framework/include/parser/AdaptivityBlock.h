#ifndef ADAPTIVITYBLOCK_H
#define ADAPTIVITYBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class AdaptivityBlock;

template<>
InputParameters validParams<AdaptivityBlock>();

class AdaptivityBlock: public ParserBlock
{
public:
  AdaptivityBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //ADAPTIVITYBLOCK_H
