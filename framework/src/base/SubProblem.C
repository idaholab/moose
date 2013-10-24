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
#include "MooseApp.h"

template<>
InputParameters validParams<SubProblem>()
{
  InputParameters params = validParams<Problem>();
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters),
    _factory(_app.getFactory()),
    _restartable_data(libMesh::n_threads())
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

std::set<SubdomainID>
SubProblem::getMaterialPropertyBlocks(const std::string & prop_name)
{
  std::set<SubdomainID> blocks;

  for(std::map<unsigned int, std::set<std::string> >::iterator it = _map_block_material_props.begin();
      it != _map_block_material_props.end();
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

  return blocks;
}

std::vector<SubdomainName>
SubProblem::getMaterialPropertyBlockNames(const std::string & prop_name)
{
  std::set<SubdomainID> blocks = getMaterialPropertyBlocks(prop_name);
  std::vector<SubdomainName> block_names;
  block_names.reserve(blocks.size());

  for (std::set<SubdomainID>::iterator it = blocks.begin(); it != blocks.end(); ++it)
  {
    std::stringstream ss;
    ss << *it;
    block_names.push_back(ss.str());
  }
  return block_names;
}

std::set<BoundaryID>
SubProblem::getMaterialPropertyBoundaryIDs(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries;

  for(std::map<unsigned int, std::set<std::string> >::iterator it = _map_boundary_material_props.begin();
      it != _map_boundary_material_props.end();
      ++it)
  {
    std::set<std::string> & prop_names = it->second;
    BoundaryID boundary = it->first;

    for(std::set<std::string>::iterator name_it = prop_names.begin();
        name_it != prop_names.end();
        ++name_it)
    {
      if(*name_it == prop_name)
        boundaries.insert(boundary);
    }
  }

  return boundaries;
}

std::vector<BoundaryName>
SubProblem::getMaterialPropertyBoundaryNames(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries = getMaterialPropertyBoundaryIDs(prop_name);
  std::vector<BoundaryName> boundary_names;
  boundary_names.reserve(boundaries.size());

  for (std::set<BoundaryID>::iterator it = boundaries.begin(); it != boundaries.end(); ++it)
  {
    std::stringstream ss;
    ss << *it;
    boundary_names.push_back(ss.str());
  }
  return boundary_names;
}

void
SubProblem::storeMatPropName(SubdomainID block_id, const std::string & name)
{
  _map_block_material_props[block_id].insert(name);
}

void
SubProblem::storeMatPropName(BoundaryID boundary_id, const std::string & name)
{
  _map_boundary_material_props[boundary_id].insert(name);
}

void
SubProblem::storeDelayedCheckMatProp(SubdomainID block_id, const std::string & name)
{
  _map_block_material_props_check[block_id].insert(name);
}

void
SubProblem::storeDelayedCheckMatProp(BoundaryID boundary_id, const std::string & name)
{
  _map_boundary_material_props_check[boundary_id].insert(name);
}

void
SubProblem::checkBlockMatProps()
{
  checkMatProps(_map_block_material_props, _map_block_material_props_check, "block");
}

void
SubProblem::checkBoundaryMatProps()
{
  checkMatProps(_map_boundary_material_props, _map_boundary_material_props_check, "boundary");
}


DiracKernelInfo &
SubProblem::diracKernelInfo()
{
  return _dirac_kernel_info;
}

Real
SubProblem::finalNonlinearResidual()
{
  return 0;
}

unsigned int
SubProblem::nNonlinearIterations()
{
  return 0;
}

unsigned int
SubProblem::nLinearIterations()
{
  return 0;
}

void
SubProblem::meshChanged()
{
  mooseError("This system does not support changing the mesh");
}

void
SubProblem::checkMatProps(std::map<unsigned int, std::set<std::string> > & props,
                          std::map<unsigned int, std::set<std::string> > & check_props,
                          std::string type)
{

  // Set flag for type: block/boundary
  bool block_type;
  if (type.compare("block") == 0)
    block_type = true;
  else if (type.compare("boundary") == 0)
    block_type = false;
  else
    mooseError("Unknown type argument, it must be 'block' or 'boundary'");

  // Variable for storing the value for ANY_BLOCK_ID/ANY_BOUNDARY_ID
  int any_id;

  // Variable for storing all available blocks/boundaries from the mesh
  std::set<int> all_ids;

  // Define the id variables based on the type of material checking
  if (block_type)
  {
    any_id = Moose::ANY_BLOCK_ID;
    all_ids.insert(mesh().meshSubdomains().begin(), mesh().meshSubdomains().end());
  }
  else
  {
    any_id = Moose::ANY_BOUNDARY_ID;
    all_ids.insert(mesh().getBoundaryIDs().begin(), mesh().getBoundaryIDs().end());
  }

  // Loop through the properties to check
  for (std::map<unsigned int, std::set<std::string> >::const_iterator check_it = check_props.begin();
       check_it != check_props.end(); ++check_it)
  {
    // The current id for the property being checked (BoundaryID || BlockID)
    int check_id = check_it->first;

    // Get the name of the block/boundary (for error reporting)
    std::string check_name;
    if (block_type)
    {
      // TODO: Put a better a interface in MOOSE
      std::map<subdomain_id_type, std::string> & name_map = mesh().getMesh().set_subdomain_name_map();
      std::map<subdomain_id_type, std::string>::const_iterator pos = name_map.find(check_id);
      if (pos != name_map.end())
        check_name = pos->second;
    }
    else
      check_name = mesh().getMesh().boundary_info->sideset_name(check_id);

    // Create a name if it doesn't exist
    if (check_name.empty())
    {
      std::ostringstream ss;
      ss << check_id;
      check_name = ss.str();
    }

    // In the case when the material being checked has an ID is set to ANY, then loop through all
    // the possible ids and verify that the material property is defined.
    if (check_id == any_id)
    {
      // Loop through all the block/boundary ids
      for (std::set<int>::const_iterator all_it = all_ids.begin(); all_it != all_ids.end(); ++all_it)
      {
        // Loop through all the stored properties
        for (std::set<std::string>::const_iterator prop_it = check_it->second.begin();
             prop_it != check_it->second.end(); ++prop_it)
        {
            // Produce an error if the material is not defined on the current block/boundary and any block/boundary
            if (props[*all_it].find(*prop_it) == props[*all_it].end() &&
                props[any_id].find(*prop_it) == props[any_id].end())
              mooseError("Material property '" + (*prop_it) + "' is not defined on " + type + " " + Moose::stringify(*all_it));
        }
      }
    }

    // If the property is contained in the map of properties, loop over the stored names for the current id and
    // check that the property is defined
    else if (props.find(check_id) != props.end())
      for (std::set<std::string>::const_iterator check_jt = check_it->second.begin(); check_jt != check_it->second.end(); ++check_jt)
      {
        std::string name = *check_jt;

        // Check if the name is contained in the map and skip over the id if it is Moose::ANY_BLOCK_ID/ANY_BOUNDARY_ID
        if (props[check_id].find(name) == props[check_id].end() && check_id != any_id)

          //if (props[id].find(name) == props[id].end())
          mooseError("Material property '" + name + "' is not defined on " + type + " " + check_name);//Moose::stringify(id));
      }
    else
      mooseError("No material defined on " + type + " " + check_name); //Moose::stringify(id));
  }
}
