#ifndef TRANSIENTBLOCK_H
#define TRANSIENTBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
class TransientBlock;

template<>
InputParameters validParams<TransientBlock>();

class TransientBlock: public ParserBlock
{
public:
  TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

  void setOutOfOrderTransientParams(Parameters & params) const;
};

  

#endif //TRANSIENTBLOCK_H
