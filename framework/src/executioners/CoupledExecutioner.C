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

#include "CoupledExecutioner.h"
#include "CoupledProblem.h"
#include "Factory.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Parser.h"
#include "Action.h"
#include "ActionFactory.h"


template<>
InputParameters validParams<CoupledExecutioner>()
{
  InputParameters params = validParams<Executioner>();

  return params;
}


CoupledExecutioner::CoupledExecutioner(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(NULL)
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam("_moose_app", &_app);
  _problem = static_cast<CoupledProblem *>(_app.getFactory().create("CoupledProblem", "master_problem", params));
}

CoupledExecutioner::~CoupledExecutioner()
{
  delete _problem;
  for (unsigned int i = 0; i < _awhs.size(); i++)
  {
    _awhs[i]->clear();
    delete _awhs[i];
    delete _parsers[i];
    delete _executioners[i];
    // Note: _fe_problems are destroyed by executioners' destructors
  }

  std::map<std::string, std::vector<ProjInfo *> >::iterator vm_it = _var_mapping.begin();
  std::map<std::string, std::vector<ProjInfo *> >::iterator vm_end = _var_mapping.end();

  for(; vm_it != vm_end; ++vm_it)
  {
    std::vector<ProjInfo *> & info_vec = vm_it->second;

    for(unsigned int i=0; i<info_vec.size(); i++)
      delete info_vec[i];
  }
}

Problem &
CoupledExecutioner::problem()
{
  return *_problem;
}

void
CoupledExecutioner::addFEProblem(const std::string & name, const FileName & file_name)
{
  ActionWarehouse * awh = new ActionWarehouse(_app, _app.syntax(), _app.getActionFactory());
  Parser * parser = new Parser(_app, *awh);

  parser->parse(file_name);
  awh->build();

  _name_index[name] = _awhs.size();
  _awhs.push_back(awh);
  _parsers.push_back(parser);
}

void
CoupledExecutioner::addVariableAction(const std::string & task, ActionWarehouse & src, const std::string & src_var_name, ActionWarehouse & dest, const std::string & dest_var_name)
{
  // first, try to find if the destination warehouse already has the variable we are going to add
  bool dest_var_exists = false;
  for (ActionIterator ai = dest.actionBlocksWithActionBegin(task); ai != dest.actionBlocksWithActionEnd(task); ai++)
  {
    Action * action = *ai;
    if (action->getShortName() == dest_var_name)
    {
      dest_var_exists = true;
      break;
    }
  }

  if (!dest_var_exists)
  {
    for (ActionIterator ai = src.actionBlocksWithActionBegin(task); ai != src.actionBlocksWithActionEnd(task); ai++)
    {
      Action * action = *ai;
      if (action->getShortName() == src_var_name)
      {
        // take the action params and change them to create an aux variable
        InputParameters src_params = action->parameters();
        InputParameters params = _app.getActionFactory().getValidParams("AddAuxVariableAction");
        params.addPrivateParam("_moose_app", &_app);
        std::string dest_name("AuxVariables/" + dest_var_name);
        params.set<Parser *>("parser") = NULL;                    // set parser to NULL, since this action was not create by a parser
        params.set<ActionWarehouse *>("awh") = &dest;             // move the action into destination action warehouse
        params.set<MooseEnum>("family") = src_params.get<MooseEnum>("family");
        params.set<MooseEnum>("order") = src_params.get<MooseEnum>("order");

        Action * dest_action = _app.getActionFactory().create("AddAuxVariableAction", dest_name, params);
        mooseAssert (dest_action != NULL, std::string("Action AddAuxVariableAction not created"));
        dest.addActionBlock(dest_action);
      }
    }
  }
}

void
CoupledExecutioner::addCoupledVariable(const std::string & dest, const std::string & dest_var_name, const std::string & src, const std::string & src_var_name)
{
  std::map<std::string, unsigned int>::iterator ini = _name_index.find(src);
  if (ini == _name_index.end())
    mooseError("Non-existent problem name '" + src + "'.");

  // TODO: the type of projection will determine the type of the variable we need to create in the target problem.

  unsigned int isrc = ini->second;
  unsigned int idest = _name_index[dest];

  // FIXME: we do not handle the case where the variable with the same name exists as nonlinear in one problem and as an auxiliary in another
  addVariableAction("add_variable", *_awhs[isrc], src_var_name, *_awhs[idest], dest_var_name);
  addVariableAction("add_aux_variable", *_awhs[isrc], src_var_name, *_awhs[idest], dest_var_name);

  ProjInfo * vi = new ProjInfo;
  vi->src = src;
  vi->src_var = src_var_name;
  vi->dest_var = dest_var_name;
  _var_mapping[dest].push_back(vi);
}

void
CoupledExecutioner::build()
{
  unsigned int n = _awhs.size();
  _executioners.resize(n);
  _fe_problems.resize(n);

  for (unsigned int i = 0; i < n; i++)
  {
    _awhs[i]->executeAllActions();
    _executioners[i] = _awhs[i]->executioner();
    _fe_problems[i] = _awhs[i]->problem();
  }

  // build an inverse map (problem ptr -> name)
  for (std::map<std::string, unsigned int>::iterator it = _name_index.begin(); it != _name_index.end(); ++it)
    _fep_mapping[_fe_problems[it->second]] = it->first;
}

void
CoupledExecutioner::projectVariables(FEProblem & fep)
{
  std::string dest = _fep_mapping[&fep];
  if (_var_mapping.find(dest) == _var_mapping.end())
    return;

  std::vector<ProjInfo *> & proj_list = _var_mapping[dest];
  for (std::vector<ProjInfo *>::iterator it = proj_list.begin(); it != proj_list.end(); ++it)
  {
    ProjInfo * pi = *it;

    unsigned int isrc = _name_index[pi->src];
    FEProblem * src_fep = _fe_problems[isrc];
    MooseVariable & src_mv = src_fep->getVariable(0, pi->src_var);
    SystemBase & src_sys = src_mv.sys();

    MooseVariable & dest_mv = fep.getVariable(0, pi->dest_var);
    SystemBase & dest_sys = dest_mv.sys();

    // get dof indices for source variable
    unsigned int src_vn = src_sys.system().variable_number(src_mv.name());
    std::set<unsigned int> src_var_indices;
    src_sys.system().local_dof_indices(src_vn, src_var_indices);

    // get dof indices for destination variable
    unsigned int dest_vn = dest_sys.system().variable_number(dest_mv.name());
    std::set<unsigned int> dest_var_indices;
    dest_sys.system().local_dof_indices(dest_vn, dest_var_indices);

    // NOTE: this is not very robust code. It relies on the smae node numbering and that inserting values in the set in the same order will result
    // in a std::set with the same ordering, i.e. items in src_var_indices and dest_var_indices correspond to each other.

    // copy the values from src solution vector to dest solution vector
    std::set<unsigned int>::iterator src_it = src_var_indices.begin();
    std::set<unsigned int>::iterator src_it_end = src_var_indices.end();
    std::set<unsigned int>::iterator dest_it = dest_var_indices.begin();
    for (; src_it != src_it_end; ++src_it, ++dest_it)
      dest_sys.solution().set(*dest_it, src_sys.solution()(*src_it));

    dest_sys.solution().close();
    dest_sys.update();
  }
}

FEProblem *
CoupledExecutioner::getProblemByName(const std::string & name)
{
  unsigned int i = _name_index[name];
  return _fe_problems[i];
}

Executioner *
CoupledExecutioner::getExecutionerByName(const std::string & name)
{
  unsigned int i = _name_index[name];
  return _executioners[i];
}
