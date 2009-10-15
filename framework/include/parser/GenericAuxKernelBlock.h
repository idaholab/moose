#ifndef GENERICAUXKERNELBLOCK_H
#define GENERICAUXKERNELBLOCK_H

#include "ParserBlock.h"

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H
