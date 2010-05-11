#ifndef GENERICEXECUTIONERBLOCK_H
#define GENERICEXECUTIONERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericExecutionerBlock;

template<>
InputParameters validParams<GenericExecutionerBlock>();

class GenericExecutionerBlock: public ParserBlock
{
public:
  GenericExecutionerBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICEXECUTIONERBLOCK_H
