#ifndef GENERICPOSTPROCESSORBLOCK_H
#define GENERICPOSTPROCESSORBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericPostprocessorBlock;

template<>
InputParameters validParams<GenericPostprocessorBlock>();

class GenericPostprocessorBlock: public ParserBlock
{
public:
  GenericPostprocessorBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICPOSTPROCESSORBLOCK_H
