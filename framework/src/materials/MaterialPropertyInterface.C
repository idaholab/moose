//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialPropertyInterface.h"
#include "MooseApp.h"
#include "MaterialBase.h"

InputParameters
MaterialPropertyInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam<Moose::MaterialDataType>(
      "_material_data_type"); // optionally force the type of MaterialData to utilize
  params.addParam<MaterialPropertyName>("prop_getter_suffix",
                                        "",
                                        "An optional suffix parameter that can be appended to any "
                                        "attempt to retrieve/get material properties. The suffix "
                                        "will be prepended with a '_' character.");
  return params;
}

namespace moose
{
namespace internal
{
bool
boundaryRestricted(const std::set<BoundaryID> & boundary_ids)
{
  return !boundary_ids.empty() && BoundaryRestrictable::restricted(boundary_ids);
}
}
}

MaterialPropertyInterface::MaterialPropertyInterface(const MooseObject * moose_object,
                                                     const std::set<SubdomainID> & block_ids,
                                                     const std::set<BoundaryID> & boundary_ids)
  : _mi_moose_object(*moose_object),
    _mi_params(_mi_moose_object.parameters()),
    _mi_name(_mi_params.get<std::string>("_object_name")),
    _mi_moose_object_name(_mi_params.get<std::string>("_moose_base"), _mi_name, "::"),
    _mi_feproblem(*_mi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mi_subproblem(*_mi_params.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _mi_tid(_mi_params.get<THREAD_ID>("_tid")),
    _material_data_type(getMaterialDataType(boundary_ids)),
    _material_data(_mi_feproblem.getMaterialData(_material_data_type, _mi_tid)),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _get_suffix(_mi_params.get<MaterialPropertyName>("prop_getter_suffix")),
    _mi_boundary_restricted(moose::internal::boundaryRestricted(boundary_ids)),
    _mi_block_ids(block_ids),
    _mi_boundary_ids(boundary_ids)
{
  moose_object->getMooseApp().registerInterfaceObject(*this);
}

MaterialPropertyName
MaterialPropertyInterface::getMaterialPropertyName(const std::string & name) const
{
  if (_mi_params.have_parameter<MaterialPropertyName>(name) && _mi_params.isParamValid(name))
    return _mi_params.get<MaterialPropertyName>(name);
  return name;
}

std::set<SubdomainID>
MaterialPropertyInterface::getMaterialPropertyBlocks(const std::string & name)
{
  return _mi_feproblem.getMaterialPropertyBlocks(name);
}

std::vector<SubdomainName>
MaterialPropertyInterface::getMaterialPropertyBlockNames(const std::string & name)
{
  return _mi_feproblem.getMaterialPropertyBlockNames(name);
}

std::set<BoundaryID>
MaterialPropertyInterface::getMaterialPropertyBoundaryIDs(const std::string & name)
{
  return _mi_feproblem.getMaterialPropertyBoundaryIDs(name);
}

std::vector<BoundaryName>
MaterialPropertyInterface::getMaterialPropertyBoundaryNames(const std::string & name)
{
  return _mi_feproblem.getMaterialPropertyBoundaryNames(name);
}

unsigned int
MaterialPropertyInterface::getMaxQps() const
{
  return _mi_feproblem.getMaxQps();
}

void
MaterialPropertyInterface::addConsumedPropertyName(const MooseObjectName & obj_name,
                                                   const std::string & prop_name)
{
  return _mi_feproblem.addConsumedPropertyName(obj_name, prop_name);
}

void
MaterialPropertyInterface::checkMaterialProperty(const std::string & name, const unsigned int state)
{
  if (state == 0)
  {
    // If the material property is boundary restrictable, add to the list of materials to check
    if (_mi_boundary_restricted)
      for (const auto & bnd_id : _mi_boundary_ids)
        _mi_feproblem.storeBoundaryDelayedCheckMatProp(_mi_name, bnd_id, name);

    // The default is to assume block restrictions
    else
      for (const auto & blk_ids : _mi_block_ids)
        _mi_feproblem.storeSubdomainDelayedCheckMatProp(_mi_name, blk_ids, name);
  }
}

void
MaterialPropertyInterface::markMatPropRequested(const std::string & name)
{
  _mi_feproblem.markMatPropRequested(name);
}

void
MaterialPropertyInterface::statefulPropertiesAllowed(bool stateful_allowed)
{
  _stateful_allowed = stateful_allowed;
}

MaterialBase &
MaterialPropertyInterface::getMaterial(const std::string & name)
{
  return getMaterialByName(_mi_params.get<MaterialName>(name));
}

void
MaterialPropertyInterface::checkBlockAndBoundaryCompatibility(
    std::shared_ptr<MaterialBase> discrete)
{
  // Check block compatibility
  if (!discrete->hasBlocks(_mi_block_ids))
  {
    std::ostringstream oss;
    oss << "Incompatible material and object blocks:";

    oss << "\n    " << paramErrorPrefix(discrete->parameters(), "block")
        << " material defined on blocks ";
    for (const auto & sbd_id : discrete->blockIDs())
      oss << sbd_id << ", ";

    oss << "\n    " << paramErrorPrefix(_mi_params, "block") << " object needs material on blocks ";
    for (const auto & block_id : _mi_block_ids)
      oss << block_id << ", ";

    mooseError(oss.str());
  }

  // Check boundary compatibility
  if (!discrete->hasBoundary(_mi_boundary_ids))
  {
    std::ostringstream oss;
    oss << "Incompatible material and object boundaries:";

    oss << "\n    " << paramErrorPrefix(discrete->parameters(), "boundary")
        << " material defined on boundaries ";
    for (const auto & bnd_id : discrete->boundaryIDs())
      oss << bnd_id << ", ";

    oss << "\n    " << paramErrorPrefix(_mi_params, "boundary")
        << " object needs material on boundaries ";
    for (const auto & bnd_id : _mi_boundary_ids)
      oss << bnd_id << ", ";

    mooseError(oss.str());
  }
}

MaterialBase &
MaterialPropertyInterface::getMaterialByName(const std::string & name, bool no_warn)
{
  std::shared_ptr<MaterialBase> discrete =
      _mi_feproblem.getMaterial(name, _material_data_type, _mi_tid, no_warn);

  checkBlockAndBoundaryCompatibility(discrete);
  return *discrete;
}

void
MaterialPropertyInterface::checkExecutionStage()
{
  if (_mi_feproblem.startedInitialSetup())
    mooseError("Material properties must be retrieved during object construction. This is a code "
               "problem.");
}

void
MaterialPropertyInterface::resolveOptionalProperties()
{
  for (auto & proxy : _optional_property_proxies)
    proxy->resolve(*this);
}

Moose::MaterialDataType
MaterialPropertyInterface::getMaterialDataType(const std::set<BoundaryID> & boundary_ids) const
{
  if (_mi_params.isParamValid("_material_data_type"))
    return _mi_params.get<Moose::MaterialDataType>("_material_data_type");
  if (moose::internal::boundaryRestricted(boundary_ids))
    return Moose::BOUNDARY_MATERIAL_DATA;
  return Moose::BLOCK_MATERIAL_DATA;
}
