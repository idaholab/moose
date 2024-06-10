//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D3D.h"
#include "HeatStructureCylindricalBase.h"
#include "HeatStructureFromFile3D.h"
#include "MeshAlignment2D3D.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D3D);

InputParameters
HSCoupler2D3D::validParams()
{
  InputParameters params = BoundaryBase::validParams();

  params.addRequiredParam<std::string>("heat_structure_2d", "The 2D heat structure to couple");
  params.addRequiredParam<std::string>("heat_structure_3d", "The 3D heat structure to couple");
  params.addRequiredParam<BoundaryName>("boundary_2d",
                                        "The boundary of the 2D heat structure to couple");
  params.addRequiredParam<BoundaryName>("boundary_3d",
                                        "The boundary of the 3D heat structure to couple");

  params.addParam<bool>("include_radiation", true, "Include radiation component of heat flux");
  params.addParam<FunctionName>(
      "emissivity_2d",
      "Emissivity of the 2D heat structure boundary as a function of temperature [K]");
  params.addParam<FunctionName>(
      "emissivity_3d",
      "Emissivity of the 3D heat structure boundary as a function of temperature [K]");
  params.addRequiredParam<FunctionName>("gap_thickness",
                                        "Gap thickness [m] as a function of temperature [K]");
  params.addRequiredParam<FunctionName>(
      "gap_thermal_conductivity",
      "Gap thermal conductivity [W/(m-K)] as a function of temperature [K]");
  params.addParam<FunctionName>(
      "gap_htc", 0, "Gap heat transfer coefficient [W/(m^2-K)] as a function of temperature [K]");

  params.addClassDescription("Couples a 2D heat structure boundary to a 3D heat structure boundary "
                             "using gap heat transfer.");

  return params;
}

HSCoupler2D3D::HSCoupler2D3D(const InputParameters & parameters)
  : BoundaryBase(parameters),

    _hs_name_2d(getParam<std::string>("heat_structure_2d")),
    _hs_name_3d(getParam<std::string>("heat_structure_3d")),
    _boundary_2d(getParam<BoundaryName>("boundary_2d")),
    _boundary_3d(getParam<BoundaryName>("boundary_3d")),

    _mesh_alignment(constMesh())
{
  addDependency(_hs_name_2d);
  addDependency(_hs_name_3d);
}

void
HSCoupler2D3D::setupMesh()
{
  BoundaryBase::setupMesh();

  if (hasComponentByName<HeatStructureCylindricalBase>(_hs_name_2d) &&
      hasComponentByName<HeatStructureFromFile3D>(_hs_name_3d))
  {
    const auto & hs_2d = getComponentByName<HeatStructureCylindricalBase>(_hs_name_2d);
    const auto & hs_3d = getComponentByName<HeatStructureFromFile3D>(_hs_name_3d);

    if (hs_2d.hasBoundary(_boundary_2d) && hs_3d.hasBoundary(_boundary_3d))
    {
      // Initialize the alignment mapping
      _mesh_alignment.initialize(hs_2d.getBoundaryInfo(_boundary_2d),
                                 hs_3d.getBoundaryInfo(_boundary_3d),
                                 hs_2d.getPosition(),
                                 hs_2d.getDirection());

      // Add entries to sparsity pattern for coupling
      if (_mesh_alignment.meshesAreAligned())
        for (const auto & elem_id : _mesh_alignment.getSecondaryElemIDs())
        {
          if (_mesh_alignment.hasCoupledPrimaryElemID(elem_id))
            getTHMProblem().augmentSparsity(elem_id,
                                            _mesh_alignment.getCoupledPrimaryElemID(elem_id));
        }
    }
  }
}

void
HSCoupler2D3D::check() const
{
  BoundaryBase::check();

  if (getParam<bool>("include_radiation"))
  {
    if (!(isParamValid("emissivity_2d") && isParamValid("emissivity_3d")))
      logError("If 'include_radiation' is 'true', then 'emissivity_2d' and 'emissivity_3d' are "
               "required.");
  }
  else
  {
    if (isParamValid("emissivity_2d") || isParamValid("emissivity_3d"))
      logError("If 'include_radiation' is 'false', then neither 'emissivity_2d' nor "
               "'emissivity_3d' can be specified.");
  }

  if (hasComponentByName<HeatStructureCylindricalBase>(_hs_name_2d))
  {
    const auto & hs = getComponentByName<HeatStructureCylindricalBase>(_hs_name_2d);
    if (!hs.hasBoundary(_boundary_2d))
      logError("The heat structure '",
               _hs_name_2d,
               "' does not have the boundary '",
               _boundary_2d,
               "'.");
  }
  else
    logError("There is no 2D cylindrical heat structure with the name '", _hs_name_2d, "'.");

  if (hasComponentByName<HeatStructureFromFile3D>(_hs_name_3d))
  {
    const auto & hs = getComponentByName<HeatStructureFromFile3D>(_hs_name_3d);
    if (!hs.hasBoundary(_boundary_3d))
      logError("The heat structure '",
               _hs_name_3d,
               "' does not have the boundary '",
               _boundary_3d,
               "'.");
  }
  else
    logError("There is no 3D heat structure with the name '", _hs_name_3d, "'.");

  if (hasComponentByName<HeatStructureCylindricalBase>(_hs_name_2d) &&
      hasComponentByName<HeatStructureFromFile3D>(_hs_name_3d) &&
      !_mesh_alignment.meshesAreAligned())
    logError("The meshes of the heat structures are not aligned.");

  const unsigned int needed_ad_container_size = 4 * _mesh_alignment.getMaxCouplingSize() + 6;
  if (MOOSE_AD_MAX_DOFS_PER_ELEM < needed_ad_container_size)
    logError("MOOSE must be configured with a larger AD container size (>= ",
             needed_ad_container_size,
             "). See HSCoupler2D3D's documentation for more information.");
}

void
HSCoupler2D3D::addMooseObjects()
{
  // add side UO on 2D boundary to cache temperature values by element ID
  const UserObjectName temperature_2d_uo_name = genName(name(), "2d_uo");
  {
    const std::string class_name = "StoreVariableByElemIDSideUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary_2d};
    params.set<std::vector<VariableName>>("variable") = {HeatConductionModel::TEMPERATURE};
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    // This UO needs to execute before the UO on the 3D boundary
    params.set<int>("execution_order_group") = -1;
    getTHMProblem().addUserObject(class_name, temperature_2d_uo_name, params);
  }

  // get the radius of the 2D heat structure boundary
  const auto & hs_2d = getComponentByName<HeatStructureCylindricalBase>(_hs_name_2d);
  const auto radius_2d = hs_2d.getInnerRadius() + hs_2d.getTotalWidth();

  // add side UO on 3D boundary to compute heat fluxes across each 3D boundary
  const UserObjectName hs_coupler_2d3d_uo_name = genName(name(), "3d_uo");
  {
    const std::string class_name = "HSCoupler2D3DUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary_3d};
    params.set<std::vector<VariableName>>("temperature") = {HeatConductionModel::TEMPERATURE};
    params.set<Real>("radius_2d") = radius_2d;
    if (getParam<bool>("include_radiation"))
    {
      params.set<FunctionName>("emissivity_2d") = getParam<FunctionName>("emissivity_2d");
      params.set<FunctionName>("emissivity_3d") = getParam<FunctionName>("emissivity_3d");
    }
    params.set<FunctionName>("gap_thickness") = getParam<FunctionName>("gap_thickness");
    params.set<FunctionName>("gap_thermal_conductivity") =
        getParam<FunctionName>("gap_thermal_conductivity");
    params.set<FunctionName>("gap_htc") = getParam<FunctionName>("gap_htc");
    params.set<UserObjectName>("temperature_2d_uo") = temperature_2d_uo_name;
    params.set<MeshAlignment2D3D *>("mesh_alignment") = &_mesh_alignment;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    getTHMProblem().addUserObject(class_name, hs_coupler_2d3d_uo_name, params);
  }

  // add BC on 2D boundary
  {
    const std::string class_name = "HSCoupler2D3DBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary_2d};
    params.set<UserObjectName>("hs_coupler_2d3d_uo") = hs_coupler_2d3d_uo_name;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), class_name, "2d"), params);
  }

  // add BC on 3D boundary
  {
    const std::string class_name = "HSCoupler2D3DBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary_3d};
    params.set<UserObjectName>("hs_coupler_2d3d_uo") = hs_coupler_2d3d_uo_name;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), class_name, "3d"), params);
  }
}
