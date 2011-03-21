#ifndef GENERICSTABILIZERBLOCK_H_
#define GENERICSTABILIZERBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericStabilizerBlock;

template<>
InputParameters validParams<GenericStabilizerBlock>();

class GenericStabilizerBlock: public ParserBlock
{
public:
  GenericStabilizerBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

  

#endif //GENERICSTABILIZERBLOCK_H
