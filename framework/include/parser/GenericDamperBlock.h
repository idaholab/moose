#ifndef GENERICDAMPERBLOCK_H_
#define GENERICDAMPERBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericDamperBlock;

template<>
InputParameters validParams<GenericDamperBlock>();

class GenericDamperBlock: public ParserBlock
{
public:
  GenericDamperBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

#endif //GENERICDAMPERBLOCK_H_
