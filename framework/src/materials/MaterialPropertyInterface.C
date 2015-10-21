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
#include "FEProblem.h"
#include "MooseApp.h"

// Standard construction
MaterialPropertyInterface::MaterialPropertyInterface(const InputParameters & parameters):
    _mi_name(parameters.get<std::string>("_object_name")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
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
  /* AuxKernels may be boundary or block restricted; however, they are built by the same action and task, add_aux_kernel.
     The type of material data that should be stored in the interface is not known until the object is constructed. Thus,
     the private parameter, '_material_data', cannot be set prior to the object creation. To enable the proper
     construction of AuxKernels a secondary, construction time method for setting the _material_data pointer in this interface
     exists. */

  // If the _material_data parameter exists, use it
  if (parameters.isParamValid("_material_data"))
    _material_data = parameters.get<MaterialData *>("_material_data");

  // If the _material_data parameter does not exist, figure it out based on the _block_ids and _boundary_ids parameters
  else
  {
    THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

    // Utilize boundary material if (1) _mi_boundary_ids is not empty and (2) it does not contain ANY_BOUNDARY_ID
    if (!_mi_boundary_ids.empty() && std::find(_mi_boundary_ids.begin(), _mi_boundary_ids.end(), Moose::ANY_BOUNDARY_ID) == _mi_boundary_ids.end())
      _material_data = _mi_feproblem.getBoundaryMaterialData(tid);
    else
      _material_data = _mi_feproblem.getMaterialData(tid);
  }
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
