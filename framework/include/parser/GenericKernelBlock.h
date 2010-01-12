#ifndef GENERICKERNELBLOCK_H
#define GENERICKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericKernelBlock;

template<>
InputParameters validParams<GenericKernelBlock>();

class GenericKernelBlock: public ParserBlock
{
public:
  GenericKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICKERNELBLOCK_H
