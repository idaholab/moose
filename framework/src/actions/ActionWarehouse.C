/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Parser.h"
#include "MooseObjectAction.h"
#include "InputFileFormatter.h"
#include "InputParameters.h"


ActionWarehouse::ActionWarehouse(Syntax & syntax) :
    _syntax(syntax),
    _generator_valid(false),
    _show_actions(false),
    _mesh(NULL),
    _displaced_mesh(NULL),
    _problem(NULL),
    _exreader(NULL),
    _executioner(NULL)
{
}

ActionWarehouse::~ActionWarehouse()
{
  delete _exreader;
}

void
ActionWarehouse::build()
{
  _ordered_names = _syntax.getSortedActionName();
  for (std::vector<std::string>::iterator it = _ordered_names.begin(); it != _ordered_names.end(); ++it)
    buildBuildableActions(*it);
}

void
ActionWarehouse::clear()
{
  for (std::map<std::string, std::vector<Action *> >::iterator it = _action_blocks.begin(); it != _action_blocks.end(); ++it)
  {
    for (std::vector<Action *>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt)
      delete *jt;
  }
  _action_blocks.clear();
  _generator_valid = false;
}

void
ActionWarehouse::addActionBlock(Action * blk)
{
  std::string action_name = blk->getAction();

  // Some error checking
  if (!_syntax.hasActionName(action_name))
    mooseError("A(n) " << action_name << " is not a registered action name");

  // Make sure that the ObjectAction action_name and Action action_name are consistent
  // otherwise that means that is action was built by the wrong type
  ObjectAction * moa = dynamic_cast<ObjectAction *>(blk);
  if (moa)
  {
    InputParameters mparams = moa->getObjectParams();
    if (mparams.have_parameter<std::string>("built_by_action"))
    {
      std::string moose_action_name = moa->getObjectParams().get<std::string>("built_by_action");
      if ((moose_action_name != action_name) &&  // if there is a mismatch, that means there is an object in the wrong section, unless...
          ! // NOT in the exception list...
          ((action_name == "add_aux_bc" && moose_action_name == "add_aux_kernel") ||
           (action_name == "add_postprocessor" && moose_action_name == "add_user_object") ||
           (action_name == "add_user_object" && moose_action_name == "add_postprocessor")))
        mooseError("Inconsistent Action Name detected! Action that satisfies " + action_name + " is building a MOOSE Object that normally satisfies " + moose_action_name);
    }
  }

  _action_blocks[action_name].push_back(blk);
}

ActionIterator
ActionWarehouse::actionBlocksWithActionBegin(const std::string & action_name)
{
  return _action_blocks[action_name].begin();
}

ActionIterator
ActionWarehouse::actionBlocksWithActionEnd(const std::string & action_name)
{
  return _action_blocks[action_name].end();
}

void
ActionWarehouse::buildBuildableActions(const std::string &action_name)
{
  if (_syntax.isActionRequired(action_name) && _action_blocks[action_name].empty())
  {
    bool ret_value = false;
    std::pair<std::multimap<std::string, std::string>::iterator,
              std::multimap<std::string, std::string>::iterator> range = ActionFactory::instance()->getA(action_name);
    for (std::multimap<std::string, std::string>::iterator it = range.first; it != range.second; ++it)
    {
      InputParameters params = ActionFactory::instance()->getValidParams(it->second);
      params.set<ActionWarehouse *>("awh") = this;

      if (params.areAllRequiredParamsValid())
      {
        addActionBlock(ActionFactory::instance()->create(it->second, "", params));
        ret_value = true;
      }
    }

    if (!ret_value)
      _unsatisfied_dependencies.insert(action_name);
  }
}

void
ActionWarehouse::checkUnsatisfiedActions() const
{
  std::stringstream oss;
  bool empty = true;

  for (std::set<std::string>::const_iterator i = _unsatisfied_dependencies.begin(); i != _unsatisfied_dependencies.end(); ++i)
  {
    if (_action_blocks.find(*i) == _action_blocks.end())
    {
      if (empty)
        empty = false;
      else
        oss << " ";
      oss << *i;
    }
  }

  if (!empty)
    mooseError(std::string("The following unsatisfied actions where found while setting up the MOOSE problem:\n")
               + oss.str() + "\n");
}

void
ActionWarehouse::printActionDependencySets()
{
  std::cerr << "[DBG][ACT] Ordered Actions:\n";

  const std::vector<std::set<std::string> > & ordered_names = _syntax.getSortedActionNameSet();
  for (std::vector<std::set<std::string> >::const_iterator i = ordered_names.begin(); i != ordered_names.end(); ++i)
  {
    std::cerr << "[DBG][ACT] (";
    unsigned int jj = 0;
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      if (jj++) std::cerr << ", ";
      std::cout << *j;
    }
    std::cout << ")\n";

    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      for (std::vector<Action *>::const_iterator k = _action_blocks[*j].begin(); k != _action_blocks[*j].end(); ++k)
        std::cerr << "[DBG][ACT]" << "\t" << (*k)->getAction() << "\n";
    }
  }
}

void
ActionWarehouse::executeAllActions()
{
  if (_show_actions)
  {
    std::cerr << "[DBG][ACT] Action Dependency Sets:\n";
    printActionDependencySets();
    std::cerr << "[DBG][ACT] Executing actions:" << std::endl;
  }

  for (std::vector<std::string>::iterator it = _ordered_names.begin(); it != _ordered_names.end(); ++it)
  {
    std::string action_name = *it;
    executeActionsWithAction(action_name);
  }
}

void
ActionWarehouse::executeActionsWithAction(const std::string & name)
{
  for (ActionIterator act_iter = actionBlocksWithActionBegin(name);
       act_iter != actionBlocksWithActionEnd(name);
       ++act_iter)
  {
    // Delay the InputParameters check of MOOSE based objects until just before "acting"
    // so that Meta-Actions can complete the build of parameters as necessary
    ObjectAction * obj_action = dynamic_cast<ObjectAction *>(*act_iter);
    if (obj_action != NULL)
      obj_action->getObjectParams().checkParams(obj_action->name());

    if (_show_actions)
      std::cerr << "[DBG][ACT] - " << (*act_iter)->name() << std::endl;
    (*act_iter)->act();
  }
}

void
ActionWarehouse::printInputFile(std::ostream & out)
{
  InputFileFormatter tree(false);

  std::map<std::string, std::vector<Action *> >::iterator iter;

  std::vector<Action *> ordered_actions;
  ordered_actions.clear();
  for (iter = _action_blocks.begin(); iter != _action_blocks.end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      ordered_actions.push_back(*j);

  for (std::vector<Action* >::iterator i = ordered_actions.begin();
       i != ordered_actions.end();
       ++i)
   {
    std::string name ((*i)->name());
    std::string action ((*i)->getAction());

    bool is_parent;
    if (_syntax.isAssociated(name, &is_parent) != "")
     {
      InputParameters params = (*i)->getParams();
      tree.insertNode(name, action, true, &params);

      MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(*i);
      if (moose_object_action)
       {
        InputParameters obj_params = moose_object_action->getObjectParams();
        tree.insertNode(name, action, false, &obj_params);
       }
     }
  }

  out << tree.print("");
}
