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
  params.addRequiredParam<MooseMesh *>("mesh", "The Mesh");
  params.addParam<Problem *>("parent", NULL, "This problem's parent problem (if any)");
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters),
    _parent(parameters.get<Problem *>("parent") == NULL ? this : parameters.get<Problem *>("parent")),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_parent == this ? *new EquationSystems(_mesh) : _parent->es()),
    _coord_sys(Moose::COORD_XYZ),
    _transient(false),
    _time(_eq.parameters.set<Real>("time")),
    _t_step(_eq.parameters.set<int>("t_step")),
    _dt(_eq.parameters.set<Real>("dt"))
{
  if (_parent == this)
  {
    _time = 0.0;
    _t_step = 0;
    _dt = 0;
    _dt_old = _dt;
    _eq.parameters.set<Problem *>("_problem") = this;
  }

  unsigned int n_threads = libMesh::n_threads();
  _functions.resize(n_threads);
}

SubProblem::~SubProblem()
{
  if (_parent == this)
    delete &_eq;

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
    for (std::map<std::string, Function *>::iterator it = _functions[i].begin(); it != _functions[i].end(); ++it)
      delete it->second;
}

void
SubProblem::init()
{
  _eq.init();
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

void
SubProblem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    Function * func = static_cast<Function *>(Factory::instance()->create(type, name, parameters));
    _functions[tid][name] = func;
  }
}

Function &
SubProblem::getFunction(const std::string & name, THREAD_ID tid)
{
  Function * function = _functions[tid][name];
  if (!function)
  {
    mooseError("Unable to find function " + name);
  }
  return *function;
}
