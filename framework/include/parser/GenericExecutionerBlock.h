#ifndef GENERICEXECUTIONERBLOCK_H_
#define GENERICEXECUTIONERBLOCK_H_

#include "ParserBlock.h"


class GenericExecutionerBlock : public ParserBlock
{
public:
  GenericExecutionerBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

template<>
InputParameters validParams<GenericExecutionerBlock>();

#endif //GENERICEXECUTIONERBLOCK_H
