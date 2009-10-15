#ifndef TRANSIENTBLOCK_H
#define TRANSIENTBLOCK_H

#include "ParserBlock.h"

class TransientBlock: public ParserBlock
{
public:
  TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();

  void setOutOfOrderTransientParams(Parameters & params) const;
};

  

#endif //TRANSIENTBLOCK_H
