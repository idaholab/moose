#include "AddDamperAction.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<AddDamperAction>()
{
  return validParams<MooseObjectAction>();
}

AddDamperAction::AddDamperAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddDamperAction::act() 
{
#ifdef DEBUG
  std::cerr << "Acting on AddDamperAction\n";
  std::cerr << "Damper:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
//  _moose_system.addDamper(_type, getShortName(), getClassParams());
}
