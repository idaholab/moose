//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromHeatStructure1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindricalBase.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"
#include "MooseUtils.h"

registerMooseObject("ThermalHydraulicsApp", HeatTransferFromHeatStructure1Phase);

InputParameters
HeatTransferFromHeatStructure1Phase::validParams()
{
  InputParameters params = HeatTransferFromTemperature1Phase::validParams();
  params += HSBoundaryInterface::validParams();

  params.addClassDescription("Connects a 1-phase flow channel and a heat structure");

  return params;
}

HeatTransferFromHeatStructure1Phase::HeatTransferFromHeatStructure1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters),
    HSBoundaryInterface(this),
    _fch_alignment(constMesh())
{
}

const FEType &
HeatTransferFromHeatStructure1Phase::getFEType()
{
  return HeatConductionModel::feType();
}

void
HeatTransferFromHeatStructure1Phase::preSetupMesh()
{
  if (hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    hs.setConnectedToFlowChannel();
  }
}

void
HeatTransferFromHeatStructure1Phase::setupMesh()
{
  if (hasComponentByName<HeatStructureBase>(_hs_name) && _hs_side_valid &&
      hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);

    _fch_alignment.build(hs.getBoundaryInfo(_hs_side), flow_channel.getElementIDs());

    for (auto & elem_id : flow_channel.getElementIDs())
    {
      dof_id_type nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);
      if (nearest_elem_id != DofObject::invalid_id)
        getTHMProblem().augmentSparsity(elem_id, nearest_elem_id);
    }
  }
}

void
HeatTransferFromHeatStructure1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();
  HSBoundaryInterface::check(this);

  if (hasComponentByName<HeatStructureBase>(_hs_name) &&
      hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);

    if (hs.getNumElems() != flow_channel.getNumElems())
      logError("The number of elements in component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getNumElems(),
               ", but the number of axial elements in component '",
               _hs_name,
               "' is ",
               hs.getNumElems(),
               ". They must be the same.");

    if (!MooseUtils::absoluteFuzzyEqual(hs.getLength(), flow_channel.getLength()))
      logError("The length of component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getLength(),
               ", but the length of component '",
               _hs_name,
               "' is ",
               hs.getLength(),
               ". They must be the same.");

    if (_hs_side_valid)
    {
      if (!_fch_alignment.check(flow_channel.getElementIDs()))
        logError("The centers of the elements of flow channel '",
                 _flow_channel_name,
                 "' do not align with the centers of the specified heat structure side.");
    }
  }
}

void
HeatTransferFromHeatStructure1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  // wall temperature initial condition
  if (!getTHMProblem().hasInitialConditionsFromFile() && !_app.isRestarting())
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    getTHMProblem().addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_subdomains);
  }
}

void
HeatTransferFromHeatStructure1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
  const bool is_cylindrical = dynamic_cast<const HeatStructureCylindricalBase *>(&hs) != nullptr;
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  const UserObjectName heat_flux_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "ADHeatFluxFromHeatStructure3EqnUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<FlowChannelAlignment *>("_fch_alignment") = &_fch_alignment;
    params.set<MaterialPropertyName>("T_wall") = _T_wall_name + "_coupled";
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, heat_flux_uo_name, params);
  }

  {
    const std::string class_name = "ADOneD3EqnEnergyHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    getTHMProblem().addKernel(class_name, genName(name(), "heat_flux_kernel"), params);
  }

  {
    const std::string class_name = "ADHeatFlux3EqnBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {getMasterSideName()};
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    params.set<Real>("P_hs_unit") = hs.getUnitPerimeter(_hs_side);
    params.set<unsigned int>("n_unit") = hs.getNumberOfUnits();
    params.set<bool>("hs_coord_system_is_cylindrical") = is_cylindrical;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "heat_flux_bc"), params);
  }

  // Transfer the temperature of the solid onto the flow channel
  {
    std::string class_name = "VariableValueTransferMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<MaterialPropertyName>("property_name") = _T_wall_name + "_coupled";
    params.set<BoundaryName>("secondary_boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("primary_boundary") = getMasterSideName();
    params.set<std::string>("paired_variable") = HeatConductionModel::TEMPERATURE;
    getTHMProblem().addMaterial(class_name, genName(name(), "T_wall_transfer_mat"), params);
  }

  // Transfer the temperature of the solid onto the flow channel as aux varaible for visualization
  {
    std::string class_name = "VariableValueTransferAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<std::vector<BoundaryName>>("boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("paired_boundary") = getMasterSideName();
    params.set<std::vector<VariableName>>("paired_variable") =
        std::vector<VariableName>(1, HeatConductionModel::TEMPERATURE);
    getTHMProblem().addAuxKernel(class_name, genName(name(), "T_wall_transfer"), params);
  }
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getMasterSideName() const
{
  return getHSBoundaryName(this);
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getSlaveSideName() const
{
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);
  return flow_channel.getNodesetName();
}
