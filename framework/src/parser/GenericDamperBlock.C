#include "GenericDamperBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericDamperBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericDamperBlock::GenericDamperBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericDamperBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericDamperBlock Object\n";
  std::cerr << "Damper:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
//  _moose_system.addDamper(_type, getShortName(), getClassParams());
}
