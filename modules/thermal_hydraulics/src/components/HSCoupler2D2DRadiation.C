//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D2DRadiation.h"
#include "HeatStructureCylindricalBase.h"
#include "THMMesh.h"
#include "MooseUtils.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D2DRadiation);

InputParameters
HSCoupler2D2DRadiation::validParams()
{
  InputParameters params = BoundaryBase::validParams();

  params.addRequiredParam<std::vector<std::string>>("heat_structures",
                                                    "The heat structures to couple");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundaries", "The boundaries of the heat structures to couple");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "emissivities",
      "emissivities > 0 & emissivities < 1",
      "Emissivities of each heat structure surface");
  params.addRequiredRangeCheckedParam<std::vector<std::vector<Real>>>(
      "view_factors",
      "view_factors >= 0 & view_factors <= 1",
      "The view factors between each surface, as a matrix. The row/column ordering corresponds to "
      "the ordering in 'heat_structures', with an additional row and column if "
      "'include_environment' is set to 'true'. Each row must sum to one.");
  params.addRequiredParam<bool>(
      "include_environment",
      "Whether or not to include an environment surrounding all of the surfaces. If the heat "
      "structure surfaces themselves form an enclosure, then set this parameter to 'false'.");
  params.addParam<Real>(
      "T_environment",
      "Environment temperature [K]. Only set if 'include_environment' is set to true.");

  params.addClassDescription("Couples boundaries of multiple 2D heat structures via radiation");

  return params;
}

HSCoupler2D2DRadiation::HSCoupler2D2DRadiation(const InputParameters & parameters)
  : BoundaryBase(parameters),

    _hs_names(getParam<std::vector<std::string>>("heat_structures")),
    _hs_boundaries(getParam<std::vector<BoundaryName>>("boundaries")),
    _include_environment(getParam<bool>("include_environment")),
    _n_hs(_hs_names.size()),
    _n_surfaces(_include_environment ? _n_hs + 1 : _n_hs),

    _mesh_alignment(constMesh())
{
  for (unsigned int i = 0; i < _n_hs; ++i)
    addDependency(_hs_names[i]);
}

void
HSCoupler2D2DRadiation::setupMesh()
{
  BoundaryBase::setupMesh();

  // If there is more than one heat structure, initialize mesh alignment and
  // augment the sparsity pattern
  if (_n_hs >= 2)
  {
    // get boundary info vectors for all boundaries
    std::vector<std::vector<std::tuple<dof_id_type, unsigned short int>>> boundary_infos;
    for (const auto i : index_range(_hs_names))
    {
      if (hasComponentByName<HeatStructureBase>(_hs_names[i]))
      {
        const auto & hs = getComponentByName<HeatStructureBase>(_hs_names[i]);
        if (i < _hs_boundaries.size() && hs.hasBoundary(_hs_boundaries[i]))
          boundary_infos.push_back(hs.getBoundaryInfo(_hs_boundaries[i]));
      }
    }

    // Initialize the alignment mapping
    if (hasComponentByName<HeatStructureBase>(_hs_names[0]))
    {
      const auto & hs = getComponentByName<HeatStructureBase>(_hs_names[0]);
      _mesh_alignment.initialize(boundary_infos, hs.getPosition(), hs.getDirection());
    }

    // Add entries to sparsity pattern for coupling
    if (_mesh_alignment.meshesAreAligned())
      for (const auto & primary_elem_id : _mesh_alignment.getPrimaryElemIDs())
      {
        const auto & coupled_elem_ids = _mesh_alignment.getCoupledSecondaryElemIDs(primary_elem_id);
        std::vector<dof_id_type> elem_ids;
        elem_ids.push_back(primary_elem_id);
        elem_ids.insert(elem_ids.end(), coupled_elem_ids.begin(), coupled_elem_ids.end());

        for (unsigned int i = 0; i < elem_ids.size(); ++i)
          for (unsigned int j = i + 1; j < elem_ids.size(); ++j)
            getTHMProblem().augmentSparsity(elem_ids[i], elem_ids[j]);
      }
  }
}

void
HSCoupler2D2DRadiation::check() const
{
  BoundaryBase::check();

  checkEqualSize<BoundaryName, std::string>("boundaries", "heat_structures");
  checkEqualSize<Real, std::string>("emissivities", "heat_structures");

  // check view factor matrix dimensions
  const auto & view_factors = getParam<std::vector<std::vector<Real>>>("view_factors");
  bool correct_size = true;
  if (view_factors.size() == _n_surfaces)
  {
    for (const auto i : index_range(view_factors))
      if (view_factors[i].size() != _n_surfaces)
        correct_size = false;
  }
  else
    correct_size = false;
  if (!correct_size)
    logError("The parameter 'view_factors' must be a square matrix of size ",
             _n_surfaces,
             ". For example, a size 2 matrix is provided as '0.2 0.8; 0.7 0.3'. The row/column "
             "ordering corresponds to the ordering in 'heat_structures', with an additional "
             "row/column if 'include_environment' is set to 'true'.");

  // check that all view factor matrix rows sum to one
  bool all_row_sums_unity = true;
  for (const auto i : index_range(view_factors))
    if (!MooseUtils::absoluteFuzzyEqual(
            std::accumulate(view_factors[i].begin(), view_factors[i].end(), 0.0), 1.0))
      all_row_sums_unity = false;
  if (!all_row_sums_unity)
    logError("All rows in 'view_factors' must sum to one.");

  if (_n_hs > 1 && !_mesh_alignment.meshesAreAligned())
    logError("The meshes of the heat structures are not aligned.");

  for (const auto i : index_range(_hs_names))
  {
    // for now we only allow cylindrical heat structures
    checkComponentOfTypeExistsByName<HeatStructureCylindricalBase>(_hs_names[i]);

    if (hasComponentByName<HeatStructureBase>(_hs_names[i]))
    {
      const auto & hs = getComponentByName<HeatStructureBase>(_hs_names[i]);
      if (i < _hs_boundaries.size() && !hs.hasBoundary(_hs_boundaries[i]))
        logError("The heat structure '",
                 _hs_names[i],
                 "' does not have the boundary '",
                 _hs_boundaries[i],
                 "'.");
    }
  }
}

void
HSCoupler2D2DRadiation::addMooseObjects()
{
  // add side UO on 2D boundary to cache temperature values by element ID
  const UserObjectName temperature_uo_name = genName(name(), "temperature_uo");
  {
    const std::string class_name = "StoreVariableByElemIDSideUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _hs_boundaries;
    params.set<std::vector<VariableName>>("variable") = {HeatConductionModel::TEMPERATURE};
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    // This UO needs to execute before the UO below
    params.set<int>("execution_order_group") = -1;
    getTHMProblem().addUserObject(class_name, temperature_uo_name, params);
  }

  // add side UO to compute heat fluxes across each boundary
  const UserObjectName hs_coupler_2d2d_uo_name = genName(name(), "uo");
  {
    const std::string class_name = "HSCoupler2D2DRadiationUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {_hs_boundaries[0]};
    params.applySpecificParameters(
        parameters(), {"emissivities", "view_factors", "include_environment", "T_environment"});
    params.set<UserObjectName>("temperature_uo") = temperature_uo_name;
    params.set<MeshAlignment2D2D *>("mesh_alignment") = &_mesh_alignment;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    getTHMProblem().addUserObject(class_name, hs_coupler_2d2d_uo_name, params);
  }

  // BCs
  for (unsigned int i = 0; i < _n_hs; ++i)
  {
    const auto & hs = getComponentByName<HeatStructureCylindricalBase>(_hs_names[i]);

    const std::string class_name = "HSCoupler2D2DRadiationRZBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_hs_boundaries[i]};
    params.set<UserObjectName>("hs_coupler_2d2d_uo") = hs_coupler_2d2d_uo_name;
    params.set<Point>("axis_point") = hs.getPosition();
    params.set<RealVectorValue>("axis_dir") = hs.getDirection();
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), class_name, i), params);
  }
}
