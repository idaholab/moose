#ifndef DAMPERSBLOCK_H_
#define DAMPERSBLOCK_H_

#include "ParserBlock.h"

class DampersBlock;

template<>
InputParameters validParams<DampersBlock>();

class DampersBlock: public ParserBlock
{
public:
  DampersBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

#endif //DAMPERSBLOCK_H_
