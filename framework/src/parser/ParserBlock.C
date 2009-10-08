#include "ParserBlock.h"

ParserBlock::ParserBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :_reg_id(reg_id),
   _real_id(real_id),
   _input_file(input_file)
{}

ParserBlock::~ParserBlock() 
{
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i != _children.end(); ++i) 
    delete (*i);
}

void
ParserBlock::execute()
{
  visitChildren();
}

std::string
ParserBlock::getType() const
{
  return _input_file((_real_id + "/type").c_str(), "");
}


std::string
ParserBlock::getShortName() const
{
  return _real_id.substr(_real_id.find_last_of('/')+1);
}

void
ParserBlock::visitChildren()
{
  std::vector<ParserBlock *>::iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
    (*i)->execute();
}

