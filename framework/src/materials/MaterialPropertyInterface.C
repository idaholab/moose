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
#include "MaterialPropertyInterface.h"
#include "MooseApp.h"
#include "Material.h"

template<>
InputParameters validParams<MaterialPropertyInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam<Moose::MaterialDataType>("_material_data_type"); // optionally force the type of MaterialData to utilize
  return params;
}


// Standard construction
MaterialPropertyInterface::MaterialPropertyInterface(const InputParameters & parameters):
    _mi_name(parameters.get<std::string>("_object_name")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_tid(parameters.get<THREAD_ID>("_tid")),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _mi_block_ids(_empty_block_ids),
    _mi_boundary_ids(_empty_boundary_ids),
    _mi_params(parameters)
{
  initializeMaterialPropertyInterface(parameters);
}

// Block restricted
MaterialPropertyInterface::MaterialPropertyInterface(const InputParameters & parameters, const std::set<SubdomainID> & block_ids):
    _mi_name(parameters.get<std::string>("_object_name")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_tid(parameters.get<THREAD_ID>("_tid")),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _mi_block_ids(block_ids),
    _mi_boundary_ids(_empty_boundary_ids),
    _mi_params(parameters)
{
  initializeMaterialPropertyInterface(parameters);
}

// Boundary restricted
MaterialPropertyInterface::MaterialPropertyInterface(const InputParameters & parameters, const std::set<BoundaryID> & boundary_ids):
    _mi_name(parameters.get<std::string>("_object_name")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_tid(parameters.get<THREAD_ID>("_tid")),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _mi_block_ids(_empty_block_ids),
    _mi_boundary_ids(boundary_ids),
    _mi_params(parameters)
{
  initializeMaterialPropertyInterface(parameters);
}

// Dual restricted
MaterialPropertyInterface::MaterialPropertyInterface(const InputParameters & parameters,
                                                     const std::set<SubdomainID> & block_ids,
                                                     const std::set<BoundaryID> & boundary_ids):
    _mi_name(parameters.get<std::string>("_object_name")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_tid(parameters.get<THREAD_ID>("_tid")),
    _stateful_allowed(true),
    _get_material_property_called(false),
    _mi_block_ids(block_ids),
    _mi_boundary_ids(boundary_ids),
    _mi_params(parameters)
{
  initializeMaterialPropertyInterface(parameters);
}

void
MaterialPropertyInterface::initializeMaterialPropertyInterface(const InputParameters & parameters)
{
  // Set the MaterialDataType flag
  if (parameters.isParamValid("_material_data_type"))
    _material_data_type = parameters.get<Moose::MaterialDataType>("_material_data_type");

  else if (!_mi_boundary_ids.empty() && std::find(_mi_boundary_ids.begin(), _mi_boundary_ids.end(), Moose::ANY_BOUNDARY_ID) == _mi_boundary_ids.end())
    _material_data_type = Moose::BOUNDARY_MATERIAL_DATA;

  else
    _material_data_type = Moose::BLOCK_MATERIAL_DATA;

  _material_data = _mi_feproblem.getMaterialData(_material_data_type, parameters.get<THREAD_ID>("_tid"));
}

std::string
MaterialPropertyInterface::deducePropertyName(const std::string & name)
{
  if (_mi_params.have_parameter<MaterialPropertyName>(name))
    return _mi_params.get<MaterialPropertyName>(name);
  else
    return name;
}

template<>
const MaterialProperty<Real> *
MaterialPropertyInterface::defaultMaterialProperty(const std::string & name)
{
  std::istringstream ss(name);
  Real real_value;

  // check if the string parsed cleanly into a Real number
  if (ss >> real_value && ss.eof())
  {
    MooseSharedPointer<MaterialProperty<Real> > default_property(new MaterialProperty<Real>);

    // resize to accomodate maximum number of qpoints
    unsigned int nqp = _mi_feproblem.getMaxQps();
    default_property->resize(nqp);

    // set values for all qpoints to the given default
    for (unsigned int qp = 0; qp < nqp; ++qp)
    (*default_property)[qp] = real_value;

    // add to the default property storage
    _default_real_properties.push_back(default_property);

    // return the raw pointer inside the shared pointer
    return default_property.get();
  }

  return NULL;
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

std::vector<BoundaryName>MaterialPropertyInterface::getMaterialPropertyBoundaryNames(const std::string & name)
{
  return _mi_feproblem.getMaterialPropertyBoundaryNames(name);
}

void
MaterialPropertyInterface::checkMaterialProperty(const std::string & name)
{
  // If the material property is block restrictable, add to the list of materials to check
  if (!_mi_block_ids.empty())
    for (std::set<SubdomainID>::const_iterator it = _mi_block_ids.begin(); it != _mi_block_ids.end(); ++it)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, *it, name);

  // If the material property is boundary restrictable, add to the list of materials to check
  if (!_mi_boundary_ids.empty())
    for (std::set<BoundaryID>::const_iterator it = _mi_boundary_ids.begin(); it != _mi_boundary_ids.end(); ++it)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, *it, name);
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
MaterialPropertyInterface::getMaterialByName(const std::string & name)
{
  MooseSharedPointer<Material> discrete = _mi_feproblem.getMaterial(name, _material_data_type, _mi_tid);

  // Check block compatibility
  if (!discrete->hasBlocks(_mi_block_ids))
  {
    std::ostringstream oss;
    oss << "The Material object '" << discrete->name() << "' is defined on blocks that are incompatible with the retrieving object '" << _mi_name << "':\n";
    oss << "  " << discrete->name();
    for (std::set<SubdomainID>::const_iterator it = discrete->blockIDs().begin(); it != discrete->blockIDs().end(); ++it)
      oss << " " << *it;
    oss << "\n";
    oss << "  " << _mi_name;
    for (std::set<SubdomainID>::const_iterator it = _mi_block_ids.begin(); it != _mi_block_ids.end(); ++it)
      oss << " " << *it;
    oss << "\n";
    mooseError(oss.str());
  }

  // Check boundary compatibility
  if (!discrete->hasBoundary(_mi_boundary_ids))
  {
    std::ostringstream oss;
    oss << "The Material object '" << discrete->name() << "' is defined on boundaries that are incompatible with the retrieving object '" << _mi_name << "':\n";
    oss << "  " << discrete->name();
    for (std::set<BoundaryID>::const_iterator it = discrete->boundaryIDs().begin(); it != discrete->boundaryIDs().end(); ++it)
      oss << " " << *it;
    oss << "\n";
    oss << "  " << _mi_name;
    for (std::set<BoundaryID>::const_iterator it = _mi_boundary_ids.begin(); it != _mi_boundary_ids.end(); ++it)
      oss << " " << *it;
    oss << "\n";
    mooseError(oss.str());
  }

  return *discrete;
}
