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
}

void
CheckIntegrityAction::act()
{
  _parser_handle._problem->checkProblemIntegrity();
}
