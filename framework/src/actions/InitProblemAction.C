#include "InitProblemAction.h"
#include "MProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<InitProblemAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


InitProblemAction::InitProblemAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
InitProblemAction::act()
{  
  _parser_handle._problem->init();
}
