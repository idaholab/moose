#include "CopyNodalVarsAction.h"
#include "Parser.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

#include <map>

template<>
InputParameters validParams<CopyNodalVarsAction>()
{
  return validParams<Action>();
}

CopyNodalVarsAction::CopyNodalVarsAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

void
CopyNodalVarsAction::act() 
{
  std::map<std::string, SystemBase *> action_system;
  action_system["add_variable"] = &_parser_handle._problem->getNonlinearSystem();
  action_system["add_aux_variable"] = &_parser_handle._problem->getAuxiliarySystem();
   
  for (std::map<std::string, SystemBase *>::iterator it = action_system.begin(); it != action_system.end(); ++it)
  {
    ActionIterator var_it = Moose::action_warehouse.actionBlocksWithActionBegin(it->first);
    for ( ; var_it != Moose::action_warehouse.actionBlocksWithActionEnd(it->first); ++var_it)
    {
      if (AddVariableAction * var_action = dynamic_cast<AddVariableAction *>(*var_it))
      {
        std::pair<std::string, unsigned int> init_pair = var_action->initialValuePair();
        if (init_pair.first != "")
          it->second->copyNodalValues(*_parser_handle._exreader, init_pair.first, init_pair.second);
      }
    }
  }
}
