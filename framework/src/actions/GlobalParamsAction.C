#include "GlobalParamsAction.h"

template<>
InputParameters validParams<GlobalParamsAction>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  /* GlobalParams should not have children or other standard public Action attributes */
  params.addPrivateParam<std::vector<std::string> >("active", blocks);
  params.addPrivateParam<Parser *>("parser_handle");
  return params;
}

GlobalParamsAction::GlobalParamsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{}

void
GlobalParamsAction::act() 
{
#ifdef DEBUG
  std::cerr << "Inside the GlobalParamsBlock Object\n";
#endif
}  

  
