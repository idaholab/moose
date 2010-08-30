#include "GenericDamperBlock.h"
#include "DamperFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericDamperBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericDamperBlock::GenericDamperBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(DamperFactory::instance()->getValidParams(_type));
}

void
GenericDamperBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericDamperBlock Object\n";
  std::cerr << "Damper:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
  _moose_system.addDamper(_type, getShortName(), getClassParams());
}
