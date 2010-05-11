#ifndef BCBLOCK_H
#define BCBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
//class InputParameters;
class Parser;
class BCBlock;

template<>
InputParameters validParams<BCBlock>();

class BCBlock: public ParserBlock
{
public:
  BCBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //BCBLOCK_H
