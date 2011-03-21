#ifndef ADAPTIVITYBLOCK_H_
#define ADAPTIVITYBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class AdaptivityBlock;

template<>
InputParameters validParams<AdaptivityBlock>();

class AdaptivityBlock: public ParserBlock
{
public:
  AdaptivityBlock(const std::string & name, InputParameters params);

  virtual void execute();

  unsigned int getSteps();
};

  

#endif //ADAPTIVITYBLOCK_H_
