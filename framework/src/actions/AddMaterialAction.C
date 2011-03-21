#include "AddMaterialAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddMaterialAction>()
{
  return validParams<MooseObjectAction>();
}

AddMaterialAction::AddMaterialAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
   std::cerr << "Constructing AddMaterialAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddMaterialAction::act()
{
  std::cerr << "Acting on AddMaterialAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";

  _parser_handle._problem->addMaterial(_type, getShortName(), _moose_object_pars);  
}
