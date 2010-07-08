#ifndef POSTPROCESSORSBLOCK_H
#define POSTPROCESSORSBLOCK_H

#include "ParserBlock.h"

class PostprocessorsBlock;

template<>
InputParameters validParams<PostprocessorsBlock>();

class PostprocessorsBlock: public ParserBlock
{
public:
  PostprocessorsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //POSTPROCESSORSBLOCK_H
