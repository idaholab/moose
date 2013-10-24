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

MaterialPropertyInterface::MaterialPropertyInterface(InputParameters & parameters) :
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _mi_feproblem(*parameters.get<FEProblem *>("_fe_problem")),
    _mi_block_ids(parameters.isParamValid("_block_ids") ?
                  parameters.get<std::vector<SubdomainID> >("_block_ids") : std::vector<SubdomainID>()),
    _mi_boundary_ids(parameters.isParamValid("_boundary_ids") ?
                     parameters.get<std::vector<BoundaryID> >("_boundary_ids") : std::vector<BoundaryID>())
{
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
      _mi_feproblem.storeDelayedCheckMatProp(*it, name);

  // If the material property is boundary restrictable, add to the list of materials to check
  if (!_mi_boundary_ids.empty())
    for (std::vector<BoundaryID>::iterator it = _mi_boundary_ids.begin(); it != _mi_boundary_ids.end(); ++it)
      _mi_feproblem.storeDelayedCheckMatProp(*it, name);
}
