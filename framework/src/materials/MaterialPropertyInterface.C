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
#include "Material.h"

template <>
InputParameters
validParams<MaterialPropertyInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam<Moose::MaterialDataType>(
      "_material_data_type"); // optionally force the type of MaterialData to utilize
  return params;
}

MaterialPropertyInterface::MaterialPropertyInterface(const MooseObject * moose_object)
  : MaterialPropertyInterface(moose_object, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS)
{
}

MaterialPropertyInterface::MaterialPropertyInterface(const MooseObject * moose_object,
                                                     const std::set<SubdomainID> & block_ids)
  : MaterialPropertyInterface(moose_object, block_ids, Moose::EMPTY_BOUNDARY_IDS)
{
}

MaterialPropertyInterface::MaterialPropertyInterface(const MooseObject * moose_object,
                                                     const std::set<BoundaryID> & boundary_ids)
  : MaterialPropertyInterface(moose_object, Moose::EMPTY_BLOCK_IDS, boundary_ids)
{
}

MaterialPropertyInterface::MaterialPropertyInterface(const MooseObject * moose_object,
                                                     const std::set<SubdomainID> & block_ids,
                                                     const std::set<BoundaryID> & boundary_ids)
  : _mi_params(moose_object->parameters()),
    _mi_name(_mi_params.get<std::string>("_object_name")),
    _mi_feproblem(*_mi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mi_tid(_mi_params.get<THREAD_ID>("_tid")),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _mi_boundary_restricted(!boundary_ids.empty() &&
                            BoundaryRestrictable::restricted(boundary_ids)),
    _mi_block_ids(block_ids),
    _mi_boundary_ids(boundary_ids)
{

  // Set the MaterialDataType flag
  if (_mi_params.isParamValid("_material_data_type"))
    _material_data_type = _mi_params.get<Moose::MaterialDataType>("_material_data_type");

  else if (_mi_boundary_restricted)
    _material_data_type = Moose::BOUNDARY_MATERIAL_DATA;

  else
    _material_data_type = Moose::BLOCK_MATERIAL_DATA;

  _material_data =
      _mi_feproblem.getMaterialData(_material_data_type, _mi_params.get<THREAD_ID>("_tid"));
}

std::string
MaterialPropertyInterface::deducePropertyName(const std::string & name)
{
  if (_mi_params.have_parameter<MaterialPropertyName>(name))
    return _mi_params.get<MaterialPropertyName>(name);
  else
    return name;
}

template <>
const MaterialProperty<Real> *
MaterialPropertyInterface::defaultMaterialProperty(const std::string & name)
{
  std::istringstream ss(name);
  Real real_value;

  // check if the string parsed cleanly into a Real number
  if (ss >> real_value && ss.eof())
  {
    _default_real_properties.emplace_back(libmesh_make_unique<MaterialProperty<Real>>());
    auto & default_property = _default_real_properties.back();

    // resize to accomodate maximum number obf qpoints
    auto nqp = _mi_feproblem.getMaxQps();
    default_property->resize(nqp);

    // set values for all qpoints to the given default
    for (decltype(nqp) qp = 0; qp < nqp; ++qp)
      (*default_property)[qp] = real_value;

    // return the raw pointer inside the shared pointer
    return default_property.get();
  }

  return nullptr;
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

void
MaterialPropertyInterface::checkMaterialProperty(const std::string & name)
{
  // If the material property is boundary restrictable, add to the list of materials to check
  if (_mi_boundary_restricted)
    for (const auto & bnd_id : _mi_boundary_ids)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, bnd_id, name);

  // The default is to assume block restrictions
  else
    for (const auto & blk_ids : _mi_block_ids)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, blk_ids, name);
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

Material &
MaterialPropertyInterface::getMaterial(const std::string & name)
{
  return getMaterialByName(_mi_params.get<MaterialName>(name));
}

Material &
MaterialPropertyInterface::getMaterialByName(const std::string & name, bool no_warn)
{
  std::shared_ptr<Material> discrete =
      _mi_feproblem.getMaterial(name, _material_data_type, _mi_tid, no_warn);

  // Check block compatibility
  if (!discrete->hasBlocks(_mi_block_ids))
  {
    std::ostringstream oss;
    oss << "The Material object '" << discrete->name()
        << "' is defined on blocks that are incompatible with the retrieving object '" << _mi_name
        << "':\n";
    oss << "  " << discrete->name();
    for (const auto & sbd_id : discrete->blockIDs())
      oss << " " << sbd_id;
    oss << "\n";
    oss << "  " << _mi_name;
    for (const auto & block_id : _mi_block_ids)
      oss << " " << block_id;
    oss << "\n";
    mooseError(oss.str());
  }

  // Check boundary compatibility
  if (!discrete->hasBoundary(_mi_boundary_ids))
  {
    std::ostringstream oss;
    oss << "The Material object '" << discrete->name()
        << "' is defined on boundaries that are incompatible with the retrieving object '"
        << _mi_name << "':\n";
    oss << "  " << discrete->name();
    for (const auto & bnd_id : discrete->boundaryIDs())
      oss << " " << bnd_id;
    oss << "\n";
    oss << "  " << _mi_name;
    for (const auto & bnd_id : _mi_boundary_ids)
      oss << " " << bnd_id;
    oss << "\n";
    mooseError(oss.str());
  }

  return *discrete;
}

void
MaterialPropertyInterface::checkExecutionStage()
{
  if (_mi_feproblem.startedInitialSetup())
    mooseError("Material properties must be retrieved during object construction to ensure correct "
               "problem integrity validation.");
}
