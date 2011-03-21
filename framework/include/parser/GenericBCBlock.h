#ifndef GENERICBCBLOCK_H_
#define GENERICBCBLOCK_H_

#include "ParserBlock.h"

class GenericBCBlock : public ParserBlock
{
public:
  GenericBCBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

template<>
InputParameters validParams<GenericBCBlock>();

#endif //GENERICBCBLOCK_H
