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

#include "Outputter.h"
#include "Problem.h"

Outputter::Outputter(EquationSystems & es) :
    _es(es)
{
}

Outputter::~Outputter()
{
}

#if 0
void
SyntaxFormatterInterface::printInputFile(ActionWarehouse & wh)
{
  std::map<std::string, std::vector<Action *> >::iterator iter;

  std::vector<Action *> ordered_actions;
  ordered_actions.clear();
  for (iter = wh.actionBlocks().begin(); iter != wh.actionBlocks().end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      ordered_actions.push_back(*j);

  std::sort(ordered_actions.begin(), ordered_actions.end(), Parser::InputFileSort());

  // Print it out!
  std::string prev_name = "";
  for (std::vector<Action* >::iterator i = ordered_actions.begin();
       i != ordered_actions.end();
       ++i)
  {
    std::string name ((*i)->name());

    if (ActionFactory::instance()->isParsed(name))
    {
      std::vector<InputParameters *> param_ptrs;
      (*i)->addParamsPtrs(param_ptrs);
      print(name, prev_name == "" ? NULL : &prev_name, param_ptrs);
      prev_name = name;
    }
  }

  _out << std::endl;
  _out << "[]" << std::endl;
}

void
SyntaxFormatterInterface::printSyntax(Syntax & syntax)
{
  std::string prev_name = "";
  std::vector<InputParameters *> params_ptrs(2);
  std::vector<std::pair<std::string, Syntax::ActionInfo> > all_names;

  preamble();

  for (std::multimap<std::string, Syntax::ActionInfo>::const_iterator iter = syntax.getAssociatedActions().begin();
       iter != syntax.getAssociatedActions().end(); ++iter)
  {
    Syntax::ActionInfo act_info = iter->second;
    // If the action_name is NULL that means we need to figure out which action_name
    // goes with this syntax for the purpose of building the Moose Object part of the tree.
    // We will figure this out by asking the ActionFactory for the registration info.
    if (act_info._action_name == "")
      act_info._action_name = ActionFactory::instance()->getActionName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter->first, act_info));
  }

  // Sort the Syntax
  std::sort(all_names.begin(), all_names.end(), Parser::InputFileSort());

  for (std::vector<std::pair<std::string, Syntax::ActionInfo> >::iterator act_names = all_names.begin(); act_names != all_names.end(); ++act_names)
  {
    InputParameters action_obj_params = ActionFactory::instance()->getValidParams(act_names->second._action);

    const std::string & action_name = act_names->second._action_name;
    std::string act_name = act_names->first;

    params_ptrs[0] = &action_obj_params;

    bool print_once = false;
    for (registeredMooseObjectIterator moose_obj = Factory::instance()->registeredObjectsBegin();
         moose_obj != Factory::instance()->registeredObjectsEnd();
         ++moose_obj)
    {
      InputParameters moose_obj_params = (moose_obj->second)();

      if (moose_obj_params.have_parameter<std::string>("built_by_action") &&
          (moose_obj_params.get<std::string>("built_by_action") == action_name ||
           // Print out aux_bcs which are "built_by_action" add_aux_kernel
           (action_name == "add_aux_bc" &&
            moose_obj_params.get<std::string>("built_by_action") == "add_aux_kernel")))
      {
        print_once = true;
        std::string name;
        size_t pos = 0;
        if (act_name[act_name.size()-1] == '*')
          pos = act_name.size()-1;
        else
          pos = act_name.size();

        // Executioner and Initial Condition syntax is non standard - we'll hack it here
        if (act_name == "Executioner")
          name = "Executioner";
        else if (act_name.find("InitialCondition") != std::string::npos)
          name = act_name;
        else
          name = act_name.substr(0, pos) + moose_obj->first;

        moose_obj_params.set<std::string>("type") = moose_obj->first;
        moose_obj_params.seenInInputFile("type");
        params_ptrs[1] = &moose_obj_params;

        print(name, &prev_name, params_ptrs);

        prev_name = name;
      }
    }

    if (!print_once && ActionFactory::instance()->isParsed(act_name))
    {
      params_ptrs[1] = NULL;

      print(act_name, prev_name == "" ? NULL : &prev_name, params_ptrs);

      prev_name = act_name;
    }

    // Preconditioner syntax is non standard - we'll hack them in here
    if (action_name == "preconditioning_meta_action")
    {
      for (ActionFactory::iterator act_obj = ActionFactory::instance()->begin();
           act_obj != ActionFactory::instance()->end();
           ++act_obj)
        if (act_obj->second._action_name == "add_preconditioning")
        {
          InputParameters precond_params = ActionFactory::instance()->getValidParams(act_obj->first);
          std::string block_name = SetupPreconditionerAction::getTypeString(act_obj->first);

          precond_params.set<std::string>("type") = block_name;
          precond_params.seenInInputFile("type");
          params_ptrs[1] = &precond_params;

          std::string name = act_name.substr(0, act_name.size()-1) + block_name;

          print(name, &act_name, params_ptrs);

          prev_name = name;
        }
    }

  }
  params_ptrs[0] = NULL;
  params_ptrs[1] = NULL;
  print("", &prev_name, params_ptrs);

  postscript();
}
#endif
