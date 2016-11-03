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

// MOOSE includes
#include "BoundaryRestrictable.h"
#include "Material.h"
#include "MooseMesh.h"
#include "MooseObject.h"

template<>
InputParameters validParams<BoundaryRestrictable>()
{
  // Create instance of InputParameters
  InputParameters params = emptyInputParameters();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  return params;
}

// Standard constructor
BoundaryRestrictable::BoundaryRestrictable(const InputParameters & parameters, bool nodal) :
    _bnd_feproblem(parameters.isParamValid("_fe_problem") ? parameters.get<FEProblem *>("_fe_problem") : NULL),
    _bnd_mesh(parameters.isParamValid("_mesh") ? parameters.get<MooseMesh *>("_mesh") : NULL),
    _bnd_dual_restrictable(parameters.get<bool>("_dual_restrictable")),
    _invalid_boundary_id(Moose::INVALID_BOUNDARY_ID),
    _block_ids(_empty_block_ids),
    _bnd_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _bnd_material_data(_bnd_feproblem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA, _bnd_tid)),
    _bnd_nodal(nodal),
    _current_boundary_id(_bnd_feproblem == NULL ? _invalid_boundary_id : _bnd_feproblem->getCurrentBoundaryID())
{
  initializeBoundaryRestrictable(parameters);
}

// Dual restricted constructor
BoundaryRestrictable::BoundaryRestrictable(const InputParameters & parameters, const std::set<SubdomainID> & block_ids, bool nodal) :
    _bnd_feproblem(parameters.isParamValid("_fe_problem") ? parameters.get<FEProblem *>("_fe_problem") : NULL),
    _bnd_mesh(parameters.isParamValid("_mesh") ? parameters.get<MooseMesh *>("_mesh") : NULL),
    _bnd_dual_restrictable(parameters.get<bool>("_dual_restrictable")),
    _invalid_boundary_id(Moose::INVALID_BOUNDARY_ID),
    _block_ids(block_ids),
    _bnd_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _bnd_material_data(_bnd_feproblem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA, _bnd_tid)),
    _bnd_nodal(nodal),
    _current_boundary_id(_bnd_feproblem == NULL ? _invalid_boundary_id : _bnd_feproblem->getCurrentBoundaryID())
{
  initializeBoundaryRestrictable(parameters);
}

void
BoundaryRestrictable::initializeBoundaryRestrictable(const InputParameters & parameters)
{
  // The name and id of the object
  const std::string & name = parameters.get<std::string>("_object_name");

  // If the mesh pointer is not defined, but FEProblem is, get it from there
  if (_bnd_feproblem != NULL && _bnd_mesh == NULL)
    _bnd_mesh = &_bnd_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_bnd_mesh == NULL)
    mooseError("The input parameters must contain a pointer to FEProblem via '_fe_problem' or a pointer to the MooseMesh via '_mesh'");

  // If the user supplies boundary IDs
  if (parameters.isParamValid("boundary"))
  {
    // Extract the blocks from the input
    _boundary_names = parameters.get<std::vector<BoundaryName> >("boundary");

    // Get the IDs from the supplied names
    std::vector<BoundaryID> vec_ids = _bnd_mesh->getBoundaryIDs(_boundary_names, true);

    // Store the IDs, handling ANY_BOUNDARY_ID if supplied
    if (std::find(_boundary_names.begin(), _boundary_names.end(), "ANY_BOUNDARY_ID") != _boundary_names.end())
      _bnd_ids.insert(Moose::ANY_BOUNDARY_ID);
    else
      _bnd_ids.insert(vec_ids.begin(), vec_ids.end());
  }

  // Produce error if the object is not allowed to be both block and boundary restricted
  if (!_bnd_dual_restrictable && !_bnd_ids.empty() && !_block_ids.empty())
    if (!_block_ids.empty() && _block_ids.find(Moose::ANY_BLOCK_ID) == _block_ids.end())
      mooseError("Attempted to restrict the object '" << name << "' to a boundary, but the object is already restricted by block(s)");

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

    if (_bnd_nodal)
      valid_ids = &_bnd_mesh->meshNodesetIds();
    else
      valid_ids = &_bnd_mesh->meshSidesetIds();

    std::vector<BoundaryID> diff;

    std::set_difference(_bnd_ids.begin(), _bnd_ids.end(), valid_ids->begin(), valid_ids->end(), std::back_inserter(diff));

    if (!diff.empty())
    {
      std::ostringstream msg;
      msg << "The object '" << name << "' contains the following boundary ids that do no exist on the mesh:";
      for (const auto & id : diff)
        msg << " " << id;
      mooseError(msg.str());
    }
  }
}

BoundaryRestrictable::~BoundaryRestrictable()
{
}

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
  return (unsigned int) _bnd_ids.size();
}


bool
BoundaryRestrictable::boundaryRestricted()
{
  return _bnd_ids.find(Moose::ANY_BOUNDARY_ID) == _bnd_ids.end();
}


bool
BoundaryRestrictable::hasBoundary(BoundaryName name) const
{
  // Create a vector and utilize the getBoundaryIDs function, which
  // handles the ANY_BOUNDARY_ID (getBoundaryID does not)
  return hasBoundary(_bnd_mesh->getBoundaryIDs({name}));
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryName> names) const
{
  return hasBoundary(_bnd_mesh->getBoundaryIDs(names));
}

bool
BoundaryRestrictable::hasBoundary(BoundaryID id) const
{
  if (_bnd_ids.empty() || _bnd_ids.find(Moose::ANY_BOUNDARY_ID) != _bnd_ids.end())
    return true;
  else
    return _bnd_ids.find(id) != _bnd_ids.end();
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryID> ids, TEST_TYPE type) const
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return hasBoundary(ids_set, type);
}

bool
BoundaryRestrictable::hasBoundary(std::set<BoundaryID> ids, TEST_TYPE type) const
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
BoundaryRestrictable::isBoundarySubset(std::set<BoundaryID> ids) const
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.empty() || ids.find(Moose::ANY_BOUNDARY_ID) != ids.end())
    return true;

  if (_bnd_ids.find(Moose::ANY_BOUNDARY_ID) != _bnd_ids.end())
    return std::includes(ids.begin(), ids.end(), _bnd_mesh->meshBoundaryIds().begin(), _bnd_mesh->meshBoundaryIds().end());
  else
    return std::includes(ids.begin(), ids.end(), _bnd_ids.begin(), _bnd_ids.end());
}

bool
BoundaryRestrictable::isBoundarySubset(std::vector<BoundaryID> ids) const
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
  const std::set<BoundaryID> & ids = hasBoundary(Moose::ANY_BOUNDARY_ID) ? meshBoundaryIDs() : boundaryIDs();

  // Loop over each BoundaryID for this object
  for (const auto & id : ids)
  {
    // Storage of material properties that have been DECLARED on this BoundaryID
    std::set<std::string> declared_props;

    // If boundary materials exist, populated the set of properties that were declared
    if (warehouse.hasActiveBoundaryObjects(id))
    {
      const std::vector<MooseSharedPointer<Material> > & mats = warehouse.getActiveBoundaryObjects(id);
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
