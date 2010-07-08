#include "GenericPostprocessorBlock.h"
#include "PostprocessorFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericPostprocessorBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericPostprocessorBlock::GenericPostprocessorBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(PostprocessorFactory::instance()->getValidParams(_type));
}

void
GenericPostprocessorBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericPostprocessorBlock Object\n";
  std::cerr << "Postprocessor:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
  _moose_system.addPostprocessor(_type, getShortName(), getClassParams());
}
