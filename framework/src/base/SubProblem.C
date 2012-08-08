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
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_mesh),
    _transient(false),
    _time(_eq.parameters.set<Real>("time")),
    _time_old(_eq.parameters.set<Real>("time_old")),
    _t_step(_eq.parameters.set<int>("t_step")),
    _dt(_eq.parameters.set<Real>("dt"))
{
  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _dt = 0;
  _dt_old = _dt;
  _eq.parameters.set<SubProblem *>("_subproblem") = this;

  unsigned int n_threads = libMesh::n_threads();
  _real_zero.resize(n_threads, 0.);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);

  _user_objects.resize(n_threads);
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

Moose::CoordinateSystemType
SubProblem::getCoordSystem(SubdomainID sid)
{
  std::map<SubdomainID, Moose::CoordinateSystemType>::iterator it = _coord_sys.find(sid);
  if (it != _coord_sys.end())
    return (*it).second;
  else
  {
    std::stringstream err;
    err << "Requested subdomain "
        << sid
        << " does not exist.";
    mooseError(err.str());
  }
}

void
SubProblem::setCoordSystem(const std::vector<SubdomainName> & blocks, const std::vector<std::string> & coord_sys)
{
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  if (blocks.size() == 0)
  {
    // no blocks specified -> assume the whole domain
    Moose::CoordinateSystemType coord_type = Moose::COORD_XYZ;                          // all is going to be XYZ by default
    if (coord_sys.size() == 0)
      ; // relax, do nothing
    else if (coord_sys.size() == 1)
      coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[0]);      // one system specified, the whole domain is going to have that system
    else
      mooseError("Multiple coordinate systems specified, but no blocks given.");

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
      _coord_sys[*it] = coord_type;
  }
  else
  {
    if (blocks.size() != coord_sys.size())
      mooseError("Number of blocks and coordinate systems does not match.");

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
      SubdomainID sid = _mesh.getSubdomainID(blocks[i]);
      Moose::CoordinateSystemType coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[i]);
      _coord_sys[sid] = coord_type;
    }

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
    {
      SubdomainID sid = *it;
      if (_coord_sys.find(sid) == _coord_sys.end())
        mooseError("Subdomain '" + Moose::stringify(sid) + "' does not have a coordinate system specified.");
    }
  }
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

