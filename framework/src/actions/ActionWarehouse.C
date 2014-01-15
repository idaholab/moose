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
#include "MooseMesh.h"
#include "AddVariableAction.h"
#include "AddAuxVariableAction.h"

ActionWarehouse::ActionWarehouse(MooseApp & app, Syntax & syntax, ActionFactory & factory) :
    _app(app),
    _syntax(syntax),
    _action_factory(factory),
    _generator_valid(false),
    _show_actions(false),
    _mesh(NULL),
    _displaced_mesh(NULL),
    _problem(NULL),
    _executioner(NULL)
{
}

ActionWarehouse::~ActionWarehouse()
{
}

void
ActionWarehouse::build()
{
  _ordered_names = _syntax.getSortedTask();
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
  std::string task = blk->getTask();

  // Some error checking
  if (!_syntax.hasTask(task))
    mooseError("A(n) " << task << " is not a registered action name");

  // Make sure that the ObjectAction task and Action task are consistent
  // otherwise that means that is action was built by the wrong type
  MooseObjectAction * moa = dynamic_cast<MooseObjectAction *>(blk);
  if (moa)
  {
    InputParameters mparams = moa->getObjectParams();
    if (mparams.have_parameter<std::string>("built_by_action"))
    {
      std::string moose_task = moa->getObjectParams().get<std::string>("built_by_action");
      if ((moose_task != task) &&  // if there is a mismatch, that means there is an object in the wrong section, unless...
          ! // NOT in the exception list...
          ((task == "add_aux_bc" && moose_task == "add_aux_kernel") ||
           (task == "add_postprocessor" && moose_task == "add_user_object") ||
           (task == "add_user_object" && moose_task == "add_postprocessor")))
        mooseError("Inconsistent Action Name detected (" + blk->name() + ")! Action that satisfies " + task + " is building a MOOSE Object that normally satisfies " + moose_task);
    }
  }

  _action_blocks[task].push_back(blk);
}

ActionIterator
ActionWarehouse::actionBlocksWithActionBegin(const std::string & task)
{
  return _action_blocks[task].begin();
}

ActionIterator
ActionWarehouse::actionBlocksWithActionEnd(const std::string & task)
{
  return _action_blocks[task].end();
}

const std::vector<Action *> &
ActionWarehouse::getActionsByName(const std::string & task) const
{
  return _action_blocks.at(task);
}

void
ActionWarehouse::buildBuildableActions(const std::string &task)
{
  if (_syntax.isActionRequired(task) && _action_blocks[task].empty())
  {
    bool ret_value = false;
    std::pair<std::multimap<std::string, std::string>::iterator,
              std::multimap<std::string, std::string>::iterator> range = _action_factory.getA(task);
    for (std::multimap<std::string, std::string>::iterator it = range.first; it != range.second; ++it)
    {
      InputParameters params = _action_factory.getValidParams(it->second);
      params.set<ActionWarehouse *>("awh") = this;

      if (params.areAllRequiredParamsValid())
      {
        addActionBlock(_action_factory.create(it->second, "", params));
        ret_value = true;
      }
    }

    if (!ret_value)
      _unsatisfied_dependencies.insert(task);
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
  Moose::err << "[DBG][ACT] Ordered Actions:\n";

  const std::vector<std::set<std::string> > & ordered_names = _syntax.getSortedTaskSet();
  for (std::vector<std::set<std::string> >::const_iterator i = ordered_names.begin(); i != ordered_names.end(); ++i)
  {
    Moose::err << "[DBG][ACT] (";
    unsigned int jj = 0;
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      if (jj++) Moose::err << ", ";
      Moose::out << *j;
    }
    Moose::out << ")\n";

    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      for (std::vector<Action *>::const_iterator k = _action_blocks[*j].begin(); k != _action_blocks[*j].end(); ++k)
      {
        Action * act = *k;
        Moose::err << "[DBG][ACT]" << "\t" << act->getTask();
        if (dynamic_cast<MooseObjectAction *>(act) != NULL ||
            dynamic_cast<AddVariableAction *>(act) != NULL ||
            dynamic_cast<AddAuxVariableAction *>(act) != NULL)
        {
          // print out short name only for MooseObjectActions
          std::stringstream ss;
          ss << ": " << (*k)->getShortName();
          if (ss.str().size() > 2)
            Moose::err << ss.str();
        }
        Moose::err << "\n";
      }
    }
  }
}

void
ActionWarehouse::executeAllActions()
{
  if (_show_actions)
  {
    Moose::err << "[DBG][ACT] Action Dependency Sets:\n";
    printActionDependencySets();
    Moose::err << "[DBG][ACT] Executing actions:" << std::endl;
  }

  for (std::vector<std::string>::iterator it = _ordered_names.begin(); it != _ordered_names.end(); ++it)
  {
    std::string task = *it;
    executeActionsWithAction(task);
  }
}

void
ActionWarehouse::executeActionsWithAction(const std::string & name)
{
  for (ActionIterator act_iter = actionBlocksWithActionBegin(name);
       act_iter != actionBlocksWithActionEnd(name);
       ++act_iter)
  {
    if (_show_actions)
      Moose::err << "[DBG][ACT] - " << (*act_iter)->name() << std::endl;
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
    std::string action ((*i)->getTask());

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

