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
#include "MooseVariable.h"
#include "MooseArray.h"

template <>
InputParameters
validParams<SubProblem>()
{
  InputParameters params = validParams<Problem>();
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const InputParameters & parameters)
  : Problem(parameters),
    _factory(_app.getFactory()),
    _nonlocal_cm(),
    _requires_nonlocal_coupling(false),
    _rz_coord_axis(1) // default to RZ rotation around y-axis
{
  unsigned int n_threads = libMesh::n_threads();
  _active_elemental_moose_variables.resize(n_threads);
  _has_active_elemental_moose_variables.resize(n_threads);
  _active_material_property_ids.resize(n_threads);
}

SubProblem::~SubProblem() {}

void
SubProblem::setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars,
                                             THREAD_ID tid)
{
  if (!moose_vars.empty())
  {
    _has_active_elemental_moose_variables[tid] = 1;
    _active_elemental_moose_variables[tid] = moose_vars;
  }
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
  _has_active_elemental_moose_variables[tid] = 0;
  _active_elemental_moose_variables[tid].clear();
}

void
SubProblem::setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids, THREAD_ID tid)
{
  if (!mat_prop_ids.empty())
    _active_material_property_ids[tid] = mat_prop_ids;
}

const std::set<unsigned int> &
SubProblem::getActiveMaterialProperties(THREAD_ID tid)
{
  return _active_material_property_ids[tid];
}

bool
SubProblem::hasActiveMaterialProperties(THREAD_ID tid)
{
  return !_active_material_property_ids[tid].empty();
}

void
SubProblem::clearActiveMaterialProperties(THREAD_ID tid)
{
  _active_material_property_ids[tid].clear();
}

std::set<SubdomainID>
SubProblem::getMaterialPropertyBlocks(const std::string & prop_name)
{
  std::set<SubdomainID> blocks;

  for (const auto & it : _map_block_material_props)
  {
    const std::set<std::string> & prop_names = it.second;
    std::set<std::string>::iterator name_it = prop_names.find(prop_name);
    if (name_it != prop_names.end())
      blocks.insert(it.first);
  }

  return blocks;
}

std::vector<SubdomainName>
SubProblem::getMaterialPropertyBlockNames(const std::string & prop_name)
{
  std::set<SubdomainID> blocks = getMaterialPropertyBlocks(prop_name);
  std::vector<SubdomainName> block_names;
  block_names.reserve(blocks.size());
  for (const auto & block_id : blocks)
  {
    SubdomainName name;
    if (block_id == Moose::ANY_BLOCK_ID)
      name = "ANY_BLOCK_ID";
    else
    {
      name = mesh().getMesh().subdomain_name(block_id);
      if (name.empty())
      {
        std::ostringstream oss;
        oss << block_id;
        name = oss.str();
      }
    }
    block_names.push_back(name);
  }

  return block_names;
}

// TODO: remove code duplication by templating
std::set<BoundaryID>
SubProblem::getMaterialPropertyBoundaryIDs(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries;

  for (const auto & it : _map_boundary_material_props)
  {
    const std::set<std::string> & prop_names = it.second;
    std::set<std::string>::iterator name_it = prop_names.find(prop_name);
    if (name_it != prop_names.end())
      boundaries.insert(it.first);
  }

  return boundaries;
}

std::vector<BoundaryName>
SubProblem::getMaterialPropertyBoundaryNames(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries = getMaterialPropertyBoundaryIDs(prop_name);
  std::vector<BoundaryName> boundary_names;
  boundary_names.reserve(boundaries.size());
  const BoundaryInfo & boundary_info = mesh().getMesh().get_boundary_info();

  for (const auto & bnd_id : boundaries)
  {
    BoundaryName name;
    if (bnd_id == Moose::ANY_BOUNDARY_ID)
      name = "ANY_BOUNDARY_ID";
    else
    {
      name = boundary_info.get_sideset_name(bnd_id);
      if (name.empty())
      {
        std::ostringstream oss;
        oss << bnd_id;
        name = oss.str();
      }
    }
    boundary_names.push_back(name);
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
SubProblem::storeZeroMatProp(SubdomainID block_id, const MaterialPropertyName & name)
{
  _zero_block_material_props[block_id].insert(name);
}

void
SubProblem::storeZeroMatProp(BoundaryID boundary_id, const MaterialPropertyName & name)
{
  _zero_boundary_material_props[boundary_id].insert(name);
}

void
SubProblem::storeDelayedCheckMatProp(const std::string & requestor,
                                     SubdomainID block_id,
                                     const std::string & name)
{
  _map_block_material_props_check[block_id].insert(std::make_pair(requestor, name));
}

void
SubProblem::storeDelayedCheckMatProp(const std::string & requestor,
                                     BoundaryID boundary_id,
                                     const std::string & name)
{
  _map_boundary_material_props_check[boundary_id].insert(std::make_pair(requestor, name));
}

void
SubProblem::checkBlockMatProps()
{
  checkMatProps(
      _map_block_material_props, _map_block_material_props_check, _zero_block_material_props);
}

void
SubProblem::checkBoundaryMatProps()
{
  checkMatProps(_map_boundary_material_props,
                _map_boundary_material_props_check,
                _zero_boundary_material_props);
}

void
SubProblem::markMatPropRequested(const std::string & prop_name)
{
  _material_property_requested.insert(prop_name);
}

bool
SubProblem::isMatPropRequested(const std::string & prop_name) const
{
  return _material_property_requested.find(prop_name) != _material_property_requested.end();
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

template <>
std::string
SubProblem::restrictionTypeName<SubdomainID>()
{
  return "block";
}

template <>
std::string
SubProblem::restrictionTypeName<BoundaryID>()
{
  return "boundary";
}

std::string
SubProblem::restrictionCheckName(SubdomainID check_id)
{
  // TODO: Put a better a interface in MOOSE
  std::map<subdomain_id_type, std::string> & name_map = mesh().getMesh().set_subdomain_name_map();
  std::map<subdomain_id_type, std::string>::const_iterator pos = name_map.find(check_id);
  if (pos != name_map.end())
    return pos->second;
  return "";
}

std::string
SubProblem::restrictionCheckName(BoundaryID check_id)
{
  return mesh().getMesh().get_boundary_info().sideset_name(check_id);
}

template <typename T>
void
SubProblem::checkMatProps(std::map<T, std::set<std::string>> & props,
                          std::map<T, std::multimap<std::string, std::string>> & check_props,
                          std::map<T, std::set<MaterialPropertyName>> & zero_props)
{
  // Variable for storing the value for ANY_BLOCK_ID/ANY_BOUNDARY_ID
  T any_id = mesh().getAnyID<T>();

  // Variable for storing all available blocks/boundaries from the mesh
  std::set<T> all_ids(mesh().getBlockOrBoundaryIDs<T>());

  // Loop through the properties to check
  for (const auto & check_it : check_props)
  {
    // The current id for the property being checked (BoundaryID || BlockID)
    T check_id = check_it.first;

    // In the case when the material being checked has an ID is set to ANY, then loop through all
    // the possible ids and verify that the material property is defined.
    std::set<T> check_ids = {check_id};
    if (check_id == any_id)
      check_ids = all_ids;

    // Loop through all the block/boundary ids
    for (const auto & id : check_ids)
    {
      // Loop through all the stored properties
      for (const auto & prop_it : check_it.second)
      {
        // Produce an error if the material property is not defined on the current block/boundary
        // and any block/boundary
        // and not is not a zero material property.
        if (props[id].count(prop_it.second) == 0 && props[any_id].count(prop_it.second) == 0 &&
            zero_props[id].count(prop_it.second) == 0 &&
            zero_props[any_id].count(prop_it.second) == 0)
        {
          std::string check_name = restrictionCheckName(id);
          if (check_name.empty())
            check_name = std::to_string(id);
          mooseError("Material property '",
                     prop_it.second,
                     "', requested by '",
                     prop_it.first,
                     "' is not defined on ",
                     restrictionTypeName<T>(),
                     " ",
                     check_name);
        }
      }
    }
  }
}

unsigned int
SubProblem::getAxisymmetricRadialCoord()
{
  if (_rz_coord_axis == 0)
    return 1; // if the rotation axis is x (0), then the radial direction is y (1)
  else
    return 0; // otherwise the radial direction is assumed to be x, i.e., the rotation axis is y
}

void
SubProblem::registerRestartableData(std::string name, RestartableDataValue * data, THREAD_ID tid)
{
  _app.registerRestartableData(this->name() + "/" + name, data, tid);
}

void
SubProblem::registerRecoverableData(std::string name)
{
  _app.registerRecoverableData(this->name() + "/" + name);
}
