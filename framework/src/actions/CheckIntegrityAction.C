#include "CheckIntegrityAction.h"
#include "MProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<CheckIntegrityAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


CheckIntegrityAction::CheckIntegrityAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing CheckIntegrityAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
CheckIntegrityAction::act()
{
  std::cerr << "Acting on CheckIntegrityAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
  
  _parser_handle._problem->checkProblemIntegrity();
}
