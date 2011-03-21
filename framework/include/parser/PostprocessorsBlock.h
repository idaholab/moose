#ifndef POSTPROCESSORSBLOCK_H_
#define POSTPROCESSORSBLOCK_H_

#include "ParserBlock.h"

class PostprocessorsBlock;

template<>
InputParameters validParams<PostprocessorsBlock>();

class PostprocessorsBlock: public ParserBlock
{
public:
  PostprocessorsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

#endif //POSTPROCESSORSBLOCK_H_
