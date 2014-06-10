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

#include "MaterialPropertyInterface.h"
#include "FEProblem.h"

MaterialPropertyInterface::MaterialPropertyInterface(const std::string & name, InputParameters & parameters) :
    _mi_name(name),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_block_ids(parameters.isParamValid("_block_ids") ?
                  parameters.get<std::vector<SubdomainID> >("_block_ids") : std::vector<SubdomainID>()),
    _mi_boundary_ids(parameters.isParamValid("_boundary_ids") ?
                     parameters.get<std::vector<BoundaryID> >("_boundary_ids") : std::vector<BoundaryID>()),
    _stateful_allowed(true),
    _get_material_property_called(false)
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
    for (std::vector<SubdomainID>::iterator it = _mi_block_ids.begin(); it != _mi_block_ids.end(); ++it)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, *it, name);

  // If the material property is boundary restrictable, add to the list of materials to check
  if (!_mi_boundary_ids.empty())
    for (std::vector<BoundaryID>::iterator it = _mi_boundary_ids.begin(); it != _mi_boundary_ids.end(); ++it)
      _mi_feproblem.storeDelayedCheckMatProp(_mi_name, *it, name);
}

void
MaterialPropertyInterface::statefulPropertiesAllowed(bool stateful_allowed)
{
  _stateful_allowed = stateful_allowed;
}
