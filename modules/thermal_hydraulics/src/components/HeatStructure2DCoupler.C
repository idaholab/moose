//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCoupler.h"
#include "HeatStructurePlate.h"
#include "HeatStructureCylindricalBase.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DCoupler);

InputParameters
HeatStructure2DCoupler::validParams()
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
  params.addRequiredParam<FunctionName>("heat_transfer_coefficient",
                                        "Heat transfer coefficient function [W/(m^2-K)]");

  params.addClassDescription(
      "Couples boundaries of two 2D heat structures via a heat transfer coefficient");

  return params;
}

HeatStructure2DCoupler::HeatStructure2DCoupler(const InputParameters & parameters)
  : BoundaryBase(parameters),

    _hs_names({getParam<std::string>("primary_heat_structure"),
               getParam<std::string>("secondary_heat_structure")}),
    _hs_boundaries(
        {getParam<BoundaryName>("primary_boundary"), getParam<BoundaryName>("secondary_boundary")}),

    _mesh_alignment(_mesh),
    _is_plate({false, false}),
    _is_cylindrical({false, false})
{
  addDependency(_hs_names[0]);
  addDependency(_hs_names[1]);
}

void
HeatStructure2DCoupler::init()
{
  BoundaryBase::init();

  if (hasComponentByName<HeatStructureBase>(_hs_names[0]) &&
      hasComponentByName<HeatStructureBase>(_hs_names[1]) && !_mesh.isDistributedMesh())
  {
    const HeatStructureBase & primary_hs = getComponentByName<HeatStructureBase>(_hs_names[0]);
    const HeatStructureBase & secondary_hs = getComponentByName<HeatStructureBase>(_hs_names[1]);

    if (primary_hs.hasBoundary(_hs_boundaries[0]) && secondary_hs.hasBoundary(_hs_boundaries[1]))
    {
      // Initialize the alignment mapping
      _mesh_alignment.initialize(primary_hs.getBoundaryInfo(_hs_boundaries[0]),
                                 secondary_hs.getBoundaryInfo(_hs_boundaries[1]));

      // Add entries to sparsity pattern for coupling
      for (const auto & elem_id : _mesh_alignment.getPrimaryBoundaryElemIDs())
      {
        const auto neighbor_elem_id = _mesh_alignment.getNeighborElemID(elem_id);
        if (neighbor_elem_id != DofObject::invalid_id)
          _sim.augmentSparsity(elem_id, neighbor_elem_id);
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
HeatStructure2DCoupler::check() const
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

  if (_mesh.isDistributedMesh())
    logError("HeatStructure2DCoupler does not work with a distributed mesh.");
  else if (!_mesh_alignment.meshesAreCoincident())
    logError("The primary and secondary boundaries must have coincident meshes.");

#ifndef MOOSE_GLOBAL_AD_INDEXING
  logError("HeatStructure2DCoupler only works with global AD indexing.");
#endif
}

void
HeatStructure2DCoupler::addMooseObjects()
{
  BoundaryBase::addMooseObjects();

  for (unsigned int i = 0; i < 2; i++)
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_names[i]);

    const std::string class_name =
        _is_cylindrical[i] ? "HeatStructure2DCouplerRZBC" : "HeatStructure2DCouplerBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::string>("coupled_variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_hs_boundaries[i]};
    params.set<MeshAlignment2D2D *>("_mesh_alignment") = &_mesh_alignment;
    params.set<FunctionName>("heat_transfer_coefficient") =
        getParam<FunctionName>("heat_transfer_coefficient");
    if (_is_cylindrical[i])
    {
      const HeatStructureCylindricalBase & hs_cylindrical =
          getComponentByName<HeatStructureCylindricalBase>(_hs_names[i]);

      params.set<Point>("axis_point") = hs.getPosition();
      params.set<RealVectorValue>("axis_dir") = hs.getDirection();
      params.set<Real>("offset") =
          hs_cylindrical.getInnerRadius() - hs_cylindrical.getAxialOffset();
    }
    _sim.addBoundaryCondition(class_name, genName(name(), class_name, i), params);
  }
}
