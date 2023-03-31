//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE Includes
#include "BlockRestrictable.h"

#include "FEProblem.h"
#include "Material.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "Conversion.h"

InputParameters
BlockRestrictable::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();

  // Add the user-facing 'block' input parameter
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks (ids or names) that this object will be applied");

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.have_parameter<bool>("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  // Return the parameters
  return params;
}

// Standard constructor
BlockRestrictable::BlockRestrictable(const MooseObject * moose_object, bool initialize /*=true*/)
  : _blk_dual_restrictable(moose_object->getParam<bool>("_dual_restrictable")),
    _blk_feproblem(moose_object->isParamValid("_fe_problem_base")
                       ? moose_object->getParam<FEProblemBase *>("_fe_problem_base")
                       : NULL),
    _blk_mesh(moose_object->isParamValid("_mesh") ? moose_object->getParam<MooseMesh *>("_mesh")
                                                  : NULL),
    _boundary_ids(_empty_boundary_ids),
    _blk_tid(moose_object->isParamValid("_tid") ? moose_object->getParam<THREAD_ID>("_tid") : 0),
    _blk_name(moose_object->getParam<std::string>("_object_name"))
{
  if (initialize)
    initializeBlockRestrictable(moose_object);
}

// Dual restricted constructor
BlockRestrictable::BlockRestrictable(const MooseObject * moose_object,
                                     const std::set<BoundaryID> & boundary_ids)
  : _blk_dual_restrictable(moose_object->getParam<bool>("_dual_restrictable")),
    _blk_feproblem(moose_object->isParamValid("_fe_problem_base")
                       ? moose_object->getParam<FEProblemBase *>("_fe_problem_base")
                       : NULL),
    _blk_mesh(moose_object->isParamValid("_mesh") ? moose_object->getParam<MooseMesh *>("_mesh")
                                                  : NULL),
    _boundary_ids(boundary_ids),
    _blk_tid(moose_object->isParamValid("_tid") ? moose_object->getParam<THREAD_ID>("_tid") : 0),
    _blk_name(moose_object->getParam<std::string>("_object_name"))
{
  initializeBlockRestrictable(moose_object);
}

void
BlockRestrictable::initializeBlockRestrictable(const MooseObject * moose_object)
{
  // If the mesh pointer is not defined, but FEProblemBase is, get it from there
  if (_blk_feproblem != NULL && _blk_mesh == NULL)
    _blk_mesh = &_blk_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_blk_mesh == NULL)
    mooseError("The input parameters must contain a pointer to FEProblem via '_fe_problem' or a "
               "pointer to the MooseMesh via '_mesh'");

  // Populate the MaterialData pointer
  if (_blk_feproblem != NULL)
    _blk_material_data = _blk_feproblem->getMaterialData(Moose::BLOCK_MATERIAL_DATA, _blk_tid);

  // The 'block' input is defined
  if (moose_object->isParamValid("block"))
  {
    // Extract the blocks from the input
    _blocks = moose_object->getParam<std::vector<SubdomainName>>("block");

    // Get the IDs from the supplied names
    _vec_ids = _blk_mesh->getSubdomainIDs(_blocks);

    // Store the IDs in a set, handling ANY_BLOCK_ID if supplied
    if (std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") != _blocks.end())
      _blk_ids.insert(Moose::ANY_BLOCK_ID);
    else
      _blk_ids.insert(_vec_ids.begin(), _vec_ids.end());
  }

  // When 'blocks' is not set and there is a "variable", use the blocks from the variable
  else if (moose_object->isParamValid("variable"))
  {
    std::string variable_name = moose_object->parameters().getMooseType("variable");
    if (!variable_name.empty())
      _blk_ids = _blk_feproblem
                     ->getVariable(_blk_tid,
                                   variable_name,
                                   Moose::VarKindType::VAR_ANY,
                                   Moose::VarFieldType::VAR_FIELD_ANY)
                     .activeSubdomains();
  }

  // Produce error if the object is not allowed to be both block and boundary restricted
  if (!_blk_dual_restrictable && !_boundary_ids.empty() && !_boundary_ids.empty())
    if (!_boundary_ids.empty() && _boundary_ids.find(Moose::ANY_BOUNDARY_ID) == _boundary_ids.end())
      moose_object->paramError("block",
                               "Attempted to restrict the object '",
                               _blk_name,
                               "' to a block, but the object is already restricted by boundary");

  // If no blocks were defined above, specify that it is valid on all blocks
  if (_blk_ids.empty() && !moose_object->isParamValid("boundary"))
  {
    _blk_ids.insert(Moose::ANY_BLOCK_ID);
    _blocks = {"ANY_BLOCK_ID"};
  }

  // If this object is block restricted, check that defined blocks exist on the mesh
  if (_blk_ids.find(Moose::ANY_BLOCK_ID) == _blk_ids.end())
  {
    const std::set<SubdomainID> & valid_ids = _blk_mesh->meshSubdomains();
    std::vector<SubdomainID> diff;

    std::set_difference(_blk_ids.begin(),
                        _blk_ids.end(),
                        valid_ids.begin(),
                        valid_ids.end(),
                        std::back_inserter(diff));

    if (!diff.empty())
    {
      std::ostringstream msg;
      auto sep = " ";
      msg << "the following blocks (ids) do not exist on the mesh:";
      for (const auto & id : diff)
      {
        if (_blk_name.size() > 0)
        {
          auto & name =
              _blocks.at(std::find(_vec_ids.begin(), _vec_ids.end(), id) - _vec_ids.begin());
          if (std::to_string(id) != name)
            msg << sep << name << " (" << id << ")";
          else
            msg << sep << id;
        }
        else
          msg << sep << id;
        sep = ", ";
      }
      moose_object->paramError("block", msg.str());
    }
  }

  // Get the mesh dimension for the blocks
  if (blockRestricted())
    _blk_dim = _blk_mesh->getBlocksMaxDimension(_blocks);
  else
    _blk_dim = _blk_mesh->dimension();
}

bool
BlockRestrictable::blockRestricted() const
{
  return _blk_ids.find(Moose::ANY_BLOCK_ID) == _blk_ids.end();
}

const std::vector<SubdomainName> &
BlockRestrictable::blocks() const
{
  return _blocks;
}

const std::set<SubdomainID> &
BlockRestrictable::blockIDs() const
{
  return _blk_ids;
}

unsigned int
BlockRestrictable::numBlocks() const
{
  return (unsigned int)_blk_ids.size();
}

bool
BlockRestrictable::hasBlocks(const SubdomainName & name) const
{
  // Create a vector and utilize the getSubdomainIDs function, which
  // handles the ANY_BLOCK_ID (getSubdomainID does not)
  std::vector<SubdomainName> names(1);
  names[0] = name;
  return hasBlocks(_blk_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(const std::vector<SubdomainName> & names) const
{
  return hasBlocks(_blk_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(const SubdomainID id) const
{
  if (_blk_ids.empty() || _blk_ids.find(Moose::ANY_BLOCK_ID) != _blk_ids.end())
    return true;
  else
    return _blk_ids.find(id) != _blk_ids.end();
}

bool
BlockRestrictable::hasBlocks(const std::vector<SubdomainID> & ids) const
{
  std::set<SubdomainID> ids_set(ids.begin(), ids.end());
  return hasBlocks(ids_set);
}

bool
BlockRestrictable::hasBlocks(const std::set<SubdomainID> & ids) const
{
  if (_blk_ids.empty() || _blk_ids.find(Moose::ANY_BLOCK_ID) != _blk_ids.end())
    return true;
  else
    return std::includes(_blk_ids.begin(), _blk_ids.end(), ids.begin(), ids.end());
}

bool
BlockRestrictable::isBlockSubset(const std::set<SubdomainID> & ids) const
{
  // An empty input is assumed to be ANY_BLOCK_ID
  if (ids.empty() || ids.find(Moose::ANY_BLOCK_ID) != ids.end())
    return true;

  if (_blk_ids.find(Moose::ANY_BLOCK_ID) != _blk_ids.end())
    return std::includes(ids.begin(),
                         ids.end(),
                         _blk_mesh->meshSubdomains().begin(),
                         _blk_mesh->meshSubdomains().end());
  else
    return std::includes(ids.begin(), ids.end(), _blk_ids.begin(), _blk_ids.end());
}

bool
BlockRestrictable::isBlockSubset(const std::vector<SubdomainID> & ids) const
{
  std::set<SubdomainID> ids_set(ids.begin(), ids.end());
  return isBlockSubset(ids_set);
}

const std::set<SubdomainID> &
BlockRestrictable::meshBlockIDs() const
{
  return _blk_mesh->meshSubdomains();
}

bool
BlockRestrictable::hasBlockMaterialPropertyHelper(const std::string & prop_name)
{

  // Reference to MaterialWarehouse for testing and retrieving block ids
  const MaterialWarehouse & warehouse = _blk_feproblem->getMaterialWarehouse();

  // Complete set of ids that this object is active
  const std::set<SubdomainID> & ids = blockRestricted() ? blockIDs() : meshBlockIDs();

  // Loop over each id for this object
  for (const auto & id : ids)
  {
    // Storage of material properties that have been DECLARED on this id
    std::set<std::string> declared_props;

    // If block materials exist, populated the set of properties that were declared
    if (warehouse.hasActiveBlockObjects(id))
    {
      const std::vector<std::shared_ptr<MaterialBase>> & mats = warehouse.getActiveBlockObjects(id);
      for (const auto & mat : mats)
      {
        const std::set<std::string> & mat_props = mat->getSuppliedItems();
        declared_props.insert(mat_props.begin(), mat_props.end());
      }
    }

    // If the supplied property is not in the list of properties on the current id, return false
    if (declared_props.find(prop_name) == declared_props.end())
      return false;
  }

  // If you get here the supplied property is defined on all blocks
  return true;
}

Moose::CoordinateSystemType
BlockRestrictable::getBlockCoordSystem()
{
  if (!_blk_mesh)
    mooseError("No mesh available in BlockRestrictable::checkCoordSystem()");
  if (!_blk_feproblem)
    mooseError("No problem available in BlockRestrictable::checkCoordSystem()");

  const auto & subdomains = blockRestricted() ? blockIDs() : meshBlockIDs();

  if (subdomains.empty())
    mooseError("No subdomains found in the problem.");

  // make sure all subdomains are using the same coordinate system
  auto coord_system = _blk_feproblem->getCoordSystem(*subdomains.begin());
  for (auto subdomain : subdomains)
    if (_blk_feproblem->getCoordSystem(subdomain) != coord_system)
      mooseError("This object requires all subdomains to have the same coordinate system.");

  return coord_system;
}

void
BlockRestrictable::checkVariable(const MooseVariableFieldBase & variable) const
{
  // a variable defined on all internal sides does not need this check because
  // it can be coupled with other variables in DG kernels
  if (variable.activeSubdomains().count(Moose::INTERNAL_SIDE_LOWERD_ID) > 0)
    return;

  if (!isBlockSubset(variable.activeSubdomains()))
  {
    std::string var_ids = Moose::stringify(variable.activeSubdomains(), ", ");
    std::string obj_ids = Moose::stringify(blockRestricted() ? _blk_ids : meshBlockIDs(), ", ");
    mooseError("The 'block' parameter of the object '",
               _blk_name,
               "' must be a subset of the 'block' parameter of the variable '",
               variable.name(),
               "':\n    Object '",
               _blk_name,
               "': ",
               obj_ids,
               "\n    Variable '",
               variable.name(),
               "': ",
               var_ids);
  }
}
