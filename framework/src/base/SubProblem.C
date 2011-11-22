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
}

SubProblem::~SubProblem()
{
  if (_parent == this)
    delete &_eq;
}

void
SubProblem::init()
{
  _eq.init();
  _eq.print_info();
}

std::vector<unsigned int>
SubProblem::getMaterialPropertyBlocks(const std::string prop_name)
{
  std::set<unsigned int> blocks;
  std::vector<unsigned int> blocks_vec;

  for(std::map<unsigned int, std::set<std::string> >::iterator it = _map_material_props.begin();
      it != _map_material_props.end();
      ++it)
  {
    std::set<std::string> & prop_names = it->second;
    int block = it->first;

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

  for(std::set<unsigned int>::iterator it = blocks.begin();
      it != blocks.end();
      ++it)
    blocks_vec.push_back(*it);

  return blocks_vec;
}

void
SubProblem::storeMatPropName(unsigned int block_id, const std::string & name)
{
  _map_material_props[block_id].insert(name);
}

void
SubProblem::checkMatProp(unsigned int block_id, const std::string & name)
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
