#ifndef GENERICEXECUTIONERBLOCK_H
#define GENERICEXECUTIONERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericExecutionerBlock;

template<>
InputParameters validParams<GenericExecutionerBlock>();

class GenericExecutionerBlock: public ParserBlock
{
public:
  GenericExecutionerBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICEXECUTIONERBLOCK_H
