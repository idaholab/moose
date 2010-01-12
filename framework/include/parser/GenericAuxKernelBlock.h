#ifndef GENERICAUXKERNELBLOCK_H
#define GENERICAUXKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericAuxKernelBlock;

template<>
InputParameters validParams<GenericAuxKernelBlock>();

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H
