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

#include "SubProblem.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "Function.h"

template<>
InputParameters validParams<SubProblem>()
{
  InputParameters params = validParams<Problem>();
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters)
{
  unsigned int n_threads = libMesh::n_threads();
  _real_zero.resize(n_threads, 0.);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);
  _active_elemental_moose_variables.resize(n_threads);
  _has_active_elemental_moose_variables.resize(n_threads);
}

SubProblem::~SubProblem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _zero[i].release();
    _grad_zero[i].release();
    _second_zero[i].release();
  }
  _real_zero.release();
  _zero.release();
  _grad_zero.release();
  _second_zero.release();
}

void
SubProblem::setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars, THREAD_ID tid)
{
  _has_active_elemental_moose_variables[tid] = true;
  _active_elemental_moose_variables[tid] = moose_vars;
}

const std::set<MooseVariable *> &
SubProblem::getActiveElementalMooseVariables(THREAD_ID tid)
{
  return _active_elemental_moose_variables[tid];
}

bool
SubProblem::hasActiveElementalMooseVariables(THREAD_ID tid)
{
  return _has_active_elemental_moose_variables[tid];
}

void
SubProblem::clearActiveElementalMooseVariables(THREAD_ID tid)
{
  _has_active_elemental_moose_variables[tid] = false;
  _active_elemental_moose_variables[tid].clear();
}

std::vector<SubdomainID>
SubProblem::getMaterialPropertyBlocks(const std::string prop_name)
{
  std::set<SubdomainID> blocks;
  std::vector<SubdomainID> blocks_vec;

  for(std::map<unsigned int, std::set<std::string> >::iterator it = _map_material_props.begin();
      it != _map_material_props.end();
      ++it)
  {
    std::set<std::string> & prop_names = it->second;
    SubdomainID block = it->first;

    for(std::set<std::string>::iterator name_it = prop_names.begin();
        name_it != prop_names.end();
        ++name_it)
    {
      if(*name_it == prop_name)
        blocks.insert(block);
    }
  }

  // Copy it out to a vector for convenience
  blocks_vec.reserve(blocks.size());

  for(std::set<SubdomainID>::iterator it = blocks.begin();
      it != blocks.end();
      ++it)
    blocks_vec.push_back(*it);

  return blocks_vec;
}

std::vector<SubdomainName>
SubProblem::getMaterialPropertyBlockNames(const std::string prop_name)
{
  std::vector<SubdomainID> blocks = getMaterialPropertyBlocks(prop_name);
  std::vector<SubdomainName> block_names(blocks.size());

  for (unsigned int i=0; i<blocks.size(); ++i)
  {
    std::stringstream ss;
    ss << blocks[i];
    block_names[i] = ss.str();
  }
  return block_names;
}

void
SubProblem::storeMatPropName(SubdomainID block_id, const std::string & name)
{
  _map_material_props[block_id].insert(name);
}

void
SubProblem::checkMatProp(SubdomainID block_id, const std::string & name)
{
  std::map<unsigned int, std::set<std::string> >::iterator it;
  if ((it = _map_material_props.find(block_id)) != _map_material_props.end())
  {
    std::set<std::string>::iterator jt;
    if ((jt = (*it).second.find(name)) == (*it).second.end())
      mooseError("Material property '" + name + "' is not defined on block " + Moose::stringify(block_id));
  }
  else
  {
    mooseError("No material defined on block " + Moose::stringify(block_id));
  }
}

