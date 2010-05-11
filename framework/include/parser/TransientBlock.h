#ifndef TRANSIENTBLOCK_H
#define TRANSIENTBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
class TransientBlock;

template<>
InputParameters validParams<TransientBlock>();

class TransientBlock: public ParserBlock
{
public:
  TransientBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

  void setOutOfOrderTransientParams(Parameters & params) const;
};

  

#endif //TRANSIENTBLOCK_H
