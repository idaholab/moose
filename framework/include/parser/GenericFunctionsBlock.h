#ifndef GENERICFUNCTIONSBLOCK_H_
#define GENERICFUNCTIONSBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericFunctionsBlock;

template<>
InputParameters validParams<GenericFunctionsBlock>();

/**
 * This class parses functions in the [Functions] block and creates them.
 */
class GenericFunctionsBlock : public ParserBlock
{
public:
  GenericFunctionsBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

#endif //GENERICFUNCTIONSBLOCK_H_
