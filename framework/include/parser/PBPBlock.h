#ifndef PBPBLOCK_H
#define PBPBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class PBPBlock;
template<>
InputParameters validParams<PBPBlock>();

class PBPBlock: public ParserBlock
{
public:
  PBPBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //PBPBLOCK_H
