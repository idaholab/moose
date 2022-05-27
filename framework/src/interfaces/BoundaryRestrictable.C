//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BoundaryRestrictable.h"
#include "Material.h"
#include "MooseMesh.h"
#include "MooseObject.h"

InputParameters
BoundaryRestrictable::validParams()
{

  // Create instance of InputParameters
  InputParameters params = emptyInputParameters();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      "The list of boundaries (ids or names) from the mesh where this boundary condition applies");

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  return params;
}

// Standard constructor
BoundaryRestrictable::BoundaryRestrictable(const MooseObject * moose_object, bool nodal)
  : _bnd_feproblem(moose_object->isParamValid("_fe_problem_base")
                       ? moose_object->getParam<FEProblemBase *>("_fe_problem_base")
                       : NULL),
    _bnd_mesh(moose_object->isParamValid("_mesh") ? moose_object->getParam<MooseMesh *>("_mesh")
                                                  : NULL),
    _bnd_dual_restrictable(moose_object->getParam<bool>("_dual_restrictable")),
    _block_ids(_empty_block_ids),
    _bnd_tid(moose_object->isParamValid("_tid") ? moose_object->getParam<THREAD_ID>("_tid") : 0),
    _bnd_material_data(_bnd_feproblem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA, _bnd_tid)),
    _bnd_nodal(nodal),
    _moose_object(*moose_object)
{
  initializeBoundaryRestrictable();
}

// Dual restricted constructor
BoundaryRestrictable::BoundaryRestrictable(const MooseObject * moose_object,
                                           const std::set<SubdomainID> & block_ids,
                                           bool nodal)
  : _bnd_feproblem(moose_object->isParamValid("_fe_problem_base")
                       ? moose_object->getParam<FEProblemBase *>("_fe_problem_base")
                       : NULL),
    _bnd_mesh(moose_object->isParamValid("_mesh") ? moose_object->getParam<MooseMesh *>("_mesh")
                                                  : NULL),
    _bnd_dual_restrictable(moose_object->getParam<bool>("_dual_restrictable")),
    _block_ids(block_ids),
    _bnd_tid(moose_object->isParamValid("_tid") ? moose_object->getParam<THREAD_ID>("_tid") : 0),
    _bnd_material_data(_bnd_feproblem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA, _bnd_tid)),
    _bnd_nodal(nodal),
    _moose_object(*moose_object)
{
  initializeBoundaryRestrictable();
}

void
BoundaryRestrictable::initializeBoundaryRestrictable()
{
  // The name and id of the object
  const std::string & name = _moose_object.getParam<std::string>("_object_name");

  // If the mesh pointer is not defined, but FEProblemBase is, get it from there
  if (_bnd_feproblem != NULL && _bnd_mesh == NULL)
    _bnd_mesh = &_bnd_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_bnd_mesh == NULL)
    mooseError("The input parameters must contain a pointer to FEProblemBase via '_fe_problem' or "
               "a pointer to the MooseMesh via '_mesh'");

  // If the user supplies boundary IDs
  if (_moose_object.isParamValid("boundary"))
  {
    // Extract the blocks from the input
    _boundary_names = _moose_object.getParam<std::vector<BoundaryName>>("boundary");

    // Get the IDs from the supplied names
    _vec_ids = _bnd_mesh->getBoundaryIDs(_boundary_names, true);

    // Store the IDs, handling ANY_BOUNDARY_ID if supplied
    if (std::find(_boundary_names.begin(), _boundary_names.end(), "ANY_BOUNDARY_ID") !=
        _boundary_names.end())
      _bnd_ids.insert(Moose::ANY_BOUNDARY_ID);
    else
      _bnd_ids.insert(_vec_ids.begin(), _vec_ids.end());
  }

  // Produce error if the object is not allowed to be both block and boundary restricted
  if (!_bnd_dual_restrictable && !_bnd_ids.empty() && !_block_ids.empty())
    if (!_block_ids.empty() && _block_ids.find(Moose::ANY_BLOCK_ID) == _block_ids.end())
      _moose_object.paramError("boundary",
                               "Attempted to restrict the object '",
                               name,
                               "' to a boundary, but the object is already restricted by block(s)");

  // Store ANY_BOUNDARY_ID if empty
  if (_bnd_ids.empty())
  {
    _bnd_ids.insert(Moose::ANY_BOUNDARY_ID);
    _boundary_names = {"ANY_BOUNDARY_ID"};
  }

  // If this object is block restricted, check that defined blocks exist on the mesh
  if (_bnd_ids.find(Moose::ANY_BOUNDARY_ID) == _bnd_ids.end())
  {
    const std::set<BoundaryID> * valid_ids;
    const char * message_ptr = nullptr;

    if (_bnd_nodal)
    {
      valid_ids = &_bnd_mesh->meshNodesetIds();
      message_ptr = "node sets";
    }
    else
    {
      valid_ids = &_bnd_mesh->meshSidesetIds();
      message_ptr = "side sets";
    }

    std::vector<BoundaryID> diff;

    std::set_difference(_bnd_ids.begin(),
                        _bnd_ids.end(),
                        valid_ids->begin(),
                        valid_ids->end(),
                        std::back_inserter(diff));

    if (!diff.empty())
    {
      std::ostringstream msg;
      auto sep = " ";
      msg << "the following " << message_ptr << " (ids) do not exist on the mesh:";
      for (const auto & id : diff)
      {
        if (_boundary_names.size() > 0)
        {
          auto & name = _boundary_names.at(std::find(_vec_ids.begin(), _vec_ids.end(), id) -
                                           _vec_ids.begin());
          if (std::to_string(id) != name)
            msg << sep << name << " (" << id << ")";
          else
            msg << sep << id;
        }
        else
          msg << sep << id;
        sep = ", ";
      }
      if (!_bnd_nodal)
        // Diagnostic message
        msg << "\n\nMOOSE distinguishes between \"node sets\" and \"side sets\" depending on "
               "whether \nyou are using \"Nodal\" or \"Integrated\" BCs respectively. Node sets "
               "corresponding \nto your side sets are constructed for you by default.\n\n"
               "Try setting \"Mesh/construct_side_list_from_node_list=true\" if you see this "
               "error.\n"
               "Note: If you are running with adaptivity you should prefer using side sets.";

      _moose_object.paramError("boundary", msg.str());
    }
  }
}

BoundaryRestrictable::~BoundaryRestrictable() {}

const std::set<BoundaryID> &
BoundaryRestrictable::boundaryIDs() const
{
  return _bnd_ids;
}

const std::vector<BoundaryName> &
BoundaryRestrictable::boundaryNames() const
{
  return _boundary_names;
}

unsigned int
BoundaryRestrictable::numBoundaryIDs() const
{
  return (unsigned int)_bnd_ids.size();
}

bool
BoundaryRestrictable::boundaryRestricted() const
{
  return BoundaryRestrictable::restricted(_bnd_ids);
}

bool
BoundaryRestrictable::restricted(const std::set<BoundaryID> & ids)
{
  return ids.find(Moose::ANY_BOUNDARY_ID) == ids.end();
}

bool
BoundaryRestrictable::hasBoundary(const BoundaryName & name) const
{
  // Create a vector and utilize the getBoundaryIDs function, which
  // handles the ANY_BOUNDARY_ID (getBoundaryID does not)
  return hasBoundary(_bnd_mesh->getBoundaryIDs({name}));
}

bool
BoundaryRestrictable::hasBoundary(const std::vector<BoundaryName> & names) const
{
  return hasBoundary(_bnd_mesh->getBoundaryIDs(names));
}

bool
BoundaryRestrictable::hasBoundary(const BoundaryID & id) const
{
  if (_bnd_ids.empty() || _bnd_ids.find(Moose::ANY_BOUNDARY_ID) != _bnd_ids.end())
    return true;
  else
    return _bnd_ids.find(id) != _bnd_ids.end();
}

bool
BoundaryRestrictable::hasBoundary(const std::vector<BoundaryID> & ids, TEST_TYPE type) const
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return hasBoundary(ids_set, type);
}

bool
BoundaryRestrictable::hasBoundary(const std::set<BoundaryID> & ids, TEST_TYPE type) const
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.empty() || ids.find(Moose::ANY_BOUNDARY_ID) != ids.end())
    return true;

  // All supplied IDs must match those of the object
  else if (type == ALL)
  {
    if (_bnd_ids.find(Moose::ANY_BOUNDARY_ID) != _bnd_ids.end())
      return true;
    else
      return std::includes(_bnd_ids.begin(), _bnd_ids.end(), ids.begin(), ids.end());
  }
  // Any of the supplied IDs must match those of the object
  else
  {
    // Loop through the supplied ids
    for (const auto & id : ids)
    {
      // Test the current supplied id
      bool test = hasBoundary(id);

      // If the id exists in the stored ids, then return true, otherwise
      if (test)
        return true;
    }
    return false;
  }
}

bool
BoundaryRestrictable::isBoundarySubset(const std::set<BoundaryID> & ids) const
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.empty() || ids.find(Moose::ANY_BOUNDARY_ID) != ids.end())
    return true;

  if (_bnd_ids.find(Moose::ANY_BOUNDARY_ID) != _bnd_ids.end())
    return std::includes(ids.begin(),
                         ids.end(),
                         _bnd_mesh->meshBoundaryIds().begin(),
                         _bnd_mesh->meshBoundaryIds().end());
  else
    return std::includes(ids.begin(), ids.end(), _bnd_ids.begin(), _bnd_ids.end());
}

bool
BoundaryRestrictable::isBoundarySubset(const std::vector<BoundaryID> & ids) const
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return isBoundarySubset(ids_set);
}

const std::set<BoundaryID> &
BoundaryRestrictable::meshBoundaryIDs() const
{
  return _bnd_mesh->getBoundaryIDs();
}

bool
BoundaryRestrictable::hasBoundaryMaterialPropertyHelper(const std::string & prop_name) const
{
  // Reference to MaterialWarehouse for testing and retrieving boundary ids
  const MaterialWarehouse & warehouse = _bnd_feproblem->getMaterialWarehouse();

  // Complete set of BoundaryIDs that this object is defined
  const std::set<BoundaryID> & ids =
      hasBoundary(Moose::ANY_BOUNDARY_ID) ? meshBoundaryIDs() : boundaryIDs();

  // Loop over each BoundaryID for this object
  for (const auto & id : ids)
  {
    // Storage of material properties that have been DECLARED on this BoundaryID
    std::set<std::string> declared_props;

    // If boundary materials exist, populated the set of properties that were declared
    if (warehouse.hasActiveBoundaryObjects(id))
    {
      const std::vector<std::shared_ptr<MaterialBase>> & mats =
          warehouse.getActiveBoundaryObjects(id);
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

  // If you get here the supplied property is defined on all boundaries
  return true;
}
