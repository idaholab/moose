#ifndef GENERICICBLOCK_H_
#define GENERICICBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericICBlock;

template<>
InputParameters validParams<GenericICBlock>();

class GenericICBlock: public ParserBlock
{
public:
  GenericICBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};


#endif //GENERICICBLOCK_H
