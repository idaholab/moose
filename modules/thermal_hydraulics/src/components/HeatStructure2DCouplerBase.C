//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCouplerBase.h"
#include "HeatStructurePlate.h"
#include "HeatStructureCylindricalBase.h"
#include "THMMesh.h"

InputParameters
HeatStructure2DCouplerBase::validParams()
{
  InputParameters params = BoundaryBase::validParams();

  params.addRequiredParam<std::string>("primary_heat_structure",
                                       "The first heat structure to couple");
  params.addRequiredParam<std::string>("secondary_heat_structure",
                                       "The second heat structure to couple");
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The boundary of the first heat structure to couple");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The boundary of the second heat structure to couple");

  return params;
}

HeatStructure2DCouplerBase::HeatStructure2DCouplerBase(const InputParameters & parameters)
  : BoundaryBase(parameters),

    _hs_names({getParam<std::string>("primary_heat_structure"),
               getParam<std::string>("secondary_heat_structure")}),
    _hs_boundaries(
        {getParam<BoundaryName>("primary_boundary"), getParam<BoundaryName>("secondary_boundary")}),

    _mesh_alignment(constMesh()),
    _is_plate({false, false}),
    _is_cylindrical({false, false})
{
  addDependency(_hs_names[0]);
  addDependency(_hs_names[1]);
}

void
HeatStructure2DCouplerBase::setupMesh()
{
  BoundaryBase::setupMesh();

  if (hasComponentByName<HeatStructureBase>(_hs_names[0]) &&
      hasComponentByName<HeatStructureBase>(_hs_names[1]))
  {
    const HeatStructureBase & primary_hs = getComponentByName<HeatStructureBase>(_hs_names[0]);
    const HeatStructureBase & secondary_hs = getComponentByName<HeatStructureBase>(_hs_names[1]);

    if (primary_hs.hasBoundary(_hs_boundaries[0]) && secondary_hs.hasBoundary(_hs_boundaries[1]))
    {
      // Initialize the alignment mapping
      _mesh_alignment.initialize(primary_hs.getBoundaryInfo(_hs_boundaries[0]),
                                 secondary_hs.getBoundaryInfo(_hs_boundaries[1]));

      // Add entries to sparsity pattern for coupling
      if (_mesh_alignment.meshesAreAligned())
        for (const auto & elem_id : _mesh_alignment.getPrimaryElemIDs())
        {
          if (_mesh_alignment.hasCoupledElemID(elem_id))
            getTHMProblem().augmentSparsity(elem_id, _mesh_alignment.getCoupledElemID(elem_id));
        }
    }
  }
}

void
HeatStructure2DCouplerBase::init()
{
  BoundaryBase::init();

  if (hasComponentByName<HeatStructureBase>(_hs_names[0]) &&
      hasComponentByName<HeatStructureBase>(_hs_names[1]))
  {
    const HeatStructureBase & primary_hs = getComponentByName<HeatStructureBase>(_hs_names[0]);
    const HeatStructureBase & secondary_hs = getComponentByName<HeatStructureBase>(_hs_names[1]);

    if (primary_hs.hasBoundary(_hs_boundaries[0]) && secondary_hs.hasBoundary(_hs_boundaries[1]))
    {
      // Get the heat structure types
      _hs_side_types.resize(2);
      _hs_side_types[0] = primary_hs.getExternalBoundaryType(_hs_boundaries[0]);
      _hs_side_types[1] = secondary_hs.getExternalBoundaryType(_hs_boundaries[1]);

      // Areas
      _areas.resize(2);
      _areas[0] = primary_hs.getBoundaryArea(_hs_boundaries[0]);
      _areas[1] = secondary_hs.getBoundaryArea(_hs_boundaries[1]);

      // Compute the coupling area fractions in case areas are not equal
      _coupling_area_fractions.resize(2);
      if (_areas[0] > _areas[1])
      {
        _coupling_area_fractions[0] = _areas[1] / _areas[0];
        _coupling_area_fractions[1] = 1.0;
      }
      else
      {
        _coupling_area_fractions[0] = 1.0;
        _coupling_area_fractions[1] = _areas[0] / _areas[1];
      }
    }
  }

  for (unsigned int i = 0; i < 2; i++)
  {
    if (hasComponentByName<HeatStructurePlate>(_hs_names[i]))
      _is_plate[i] = true;
    if (hasComponentByName<HeatStructureCylindricalBase>(_hs_names[i]))
      _is_cylindrical[i] = true;
  }
}

void
HeatStructure2DCouplerBase::check() const
{
  BoundaryBase::check();

  for (unsigned int i = 0; i < 2; i++)
  {
    checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_names[i]);

    if (hasComponentByName<HeatStructureBase>(_hs_names[i]))
    {
      const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_names[i]);
      if (!hs.hasBoundary(_hs_boundaries[i]))
        logError("The heat structure '",
                 _hs_names[i],
                 "' does not have the boundary '",
                 _hs_boundaries[i],
                 "'.");

      if ((!_is_plate[i]) && (!_is_cylindrical[i]))
        logError("The type of the heat structure '",
                 _hs_names[i],
                 "' is not 'HeatStructurePlate' or inherited from 'HeatStructureCylindricalBase'.");
    }
  }

  if ((_is_plate[0] && _is_cylindrical[1]) || (_is_cylindrical[0] && _is_plate[1]))
    logError("The coupled heat structures must have the same type.");
}
