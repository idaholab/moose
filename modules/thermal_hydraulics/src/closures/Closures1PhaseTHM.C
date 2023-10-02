//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Closures1PhaseTHM.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"
#include "HeatTransfer1PhaseBase.h"

registerMooseObject("ThermalHydraulicsApp", Closures1PhaseTHM);

InputParameters
Closures1PhaseTHM::validParams()
{
  InputParameters params = Closures1PhaseBase::validParams();

  MooseEnum wall_htc_closure("dittus_boelter=0 kazimi_carelli=1 lyon=2 mikityuk=3 schad=4 "
                             "weisman=5 wolf_mccarthy=6 gnielinski=7",
                             "dittus_boelter");
  params.addParam<MooseEnum>(
      "wall_htc_closure", wall_htc_closure, "Heat transfer coefficient closure");
  MooseEnum wall_ff_closure("cheng_todreas=0 churchill=1", "churchill");
  params.addParam<MooseEnum>("wall_ff_closure", wall_ff_closure, "Friction factor closure");
  params.addClassDescription("Closures for 1-phase flow channels");
  return params;
}

Closures1PhaseTHM::Closures1PhaseTHM(const InputParameters & params)
  : Closures1PhaseBase(params),
    _wall_htc_closure(getParam<MooseEnum>("wall_htc_closure").getEnum<WallHTCClosureType>()),
    _wall_ff_closure(getParam<MooseEnum>("wall_ff_closure").getEnum<WallFFClosureType>())
{
}

void
Closures1PhaseTHM::checkFlowChannel(const FlowChannelBase & /*flow_channel*/) const
{
}

void
Closures1PhaseTHM::checkHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                     const FlowChannelBase & /*flow_channel*/) const
{
}

void
Closures1PhaseTHM::addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel)
{
  const FlowChannel1Phase & flow_channel_1phase =
      dynamic_cast<const FlowChannel1Phase &>(flow_channel);

  // wall friction material
  if (flow_channel.isParamValid("f"))
    addWallFrictionFunctionMaterial(flow_channel_1phase);
  else
    addWallFFMaterial(flow_channel_1phase);

  const unsigned int n_ht_connections = flow_channel_1phase.getNumberOfHeatTransferConnections();
  if (n_ht_connections > 0)
  {

    for (unsigned int i = 0; i < n_ht_connections; i++)
    {
      // wall heat transfer coefficient material
      addWallHTCMaterial(flow_channel_1phase, i);

      // wall temperature material
      if (flow_channel.getTemperatureMode())
        addWallTemperatureFromAuxMaterial(flow_channel_1phase, i);
      else
        addTemperatureWallFromHeatFluxMaterial(flow_channel_1phase, i);
    }
  }
}
void
Closures1PhaseTHM::addWallFFMaterial(const FlowChannel1Phase & flow_channel) const
{
  switch (_wall_ff_closure)
  {
    case WallFFClosureType::CHURCHILL:
    {
      const std::string class_name = "ADWallFrictionChurchillMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
      params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
      params.set<MaterialPropertyName>("D_h") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
      params.set<MaterialPropertyName>("f_D") = FlowModelSinglePhase::FRICTION_FACTOR_DARCY;
      params.set<MaterialPropertyName>("mu") = FlowModelSinglePhase::DYNAMIC_VISCOSITY;
      params.set<Real>("roughness") = flow_channel.getParam<Real>("roughness");
      const std::string obj_name = genName(flow_channel.name(), "wall_friction_mat");
      _sim.addMaterial(class_name, obj_name, params);
      flow_channel.connectObject(params, obj_name, "roughness");
      break;
    }
    case WallFFClosureType::CHENG_TODREAS:
    {
      const std::string class_name = "ADWallFrictionChengMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<MaterialPropertyName>("f_D") = FlowModelSinglePhase::FRICTION_FACTOR_DARCY;
      params.set<Real>("PoD") = flow_channel.getParam<Real>("PoD");
      if (flow_channel.getParam<Real>("PoD") == 1.0)
      {
        mooseDoOnce(mooseWarning(
            "You are using a rod bundle correlation with the default Pitch-to-Diameter "
            "ratio value, P/D=1.0. It can be set using the PoD parameter in the corresponding "
            "FlowChannel1Phase component"));
      }
      if (flow_channel.getHeatTransferGeometry() == FlowChannelBase::EConvHeatTransGeom::PIPE)
      {
        mooseError("The Cheng-Todreas correlation was made to be used in rod bundles, your "
                   "geometry type is "
                   "PIPE, please change heat_transfer_geom to ROD_BUNDLE or HEX_ROD_BUNDLE, or "
                   "choose a correlation valid for PIPES");
      }
      else if (flow_channel.getHeatTransferGeometry() ==
               FlowChannelBase::EConvHeatTransGeom::ROD_BUNDLE)
      {
        params.set<MooseEnum>("bundle_array") = "SQUARE";
      }
      else if (flow_channel.getHeatTransferGeometry() ==
               FlowChannelBase::EConvHeatTransGeom::HEX_ROD_BUNDLE)
      {
        params.set<MooseEnum>("bundle_array") = "HEXAGONAL";
      }
      if (flow_channel.getPipeLocation() == FlowChannelBase::EPipeLocation::INTERIOR)
      {
        params.set<MooseEnum>("subchannel_type") = "INTERIOR";
      }
      else if (flow_channel.getPipeLocation() == FlowChannelBase::EPipeLocation::EDGE)
      {
        params.set<MooseEnum>("subchannel_type") = "EDGE";
      }
      else if (flow_channel.getPipeLocation() == FlowChannelBase::EPipeLocation::CORNER)
      {
        params.set<MooseEnum>("subchannel_type") = "CORNER";
      }
      const std::string obj_name = genName(flow_channel.name(), "wall_friction_mat");
      _sim.addMaterial(class_name, obj_name, params);
      break;
    }
    default:
      mooseError("Invalid WallFFClosureType");
  }
}

void
Closures1PhaseTHM::addWallHTCMaterial(const FlowChannel1Phase & flow_channel, unsigned int i) const
{

  switch (_wall_htc_closure)
  {
    case WallHTCClosureType::DITTUS_BOELTER:
    {
      const std::string class_name = "ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("D_h") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
      params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
      params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
      params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
      params.set<MaterialPropertyName>("k") = FlowModelSinglePhase::THERMAL_CONDUCTIVITY;
      params.set<MaterialPropertyName>("mu") = FlowModelSinglePhase::DYNAMIC_VISCOSITY;
      params.set<MaterialPropertyName>("cp") =
          FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE;
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);

      break;
    }
    case WallHTCClosureType::WOLF_MCCARTHY:
    {
      const std::string class_name = "ADWallHeatTransferCoefficientWolfMcCarthyMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);

      break;
    }
    case WallHTCClosureType::WEISMAN:
    {

      const std::string class_name = "ADWallHeatTransferCoefficientWeismanMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<Real>("PoD") = flow_channel.getParam<Real>("PoD");
      if (flow_channel.getParam<Real>("PoD") == 1.0)
      {
        mooseDoOnce(mooseWarning(
            "You are using a rod bundle correlation with the default Pitch-to-Diameter "
            "ratio value, P/D=1.0. It can be set using the PoD parameter in the corresponding "
            "FlowChannel1Phase component"));
      }

      if (flow_channel.getHeatTransferGeometry() == FlowChannelBase::EConvHeatTransGeom::PIPE)
      {
        mooseError("Weiman's correlation was made to be used in rod bundles, your geometry type is "
                   "PIPE, please change heat_transfer_geom to ROD_BUNDLE or HEX_ROD_BUNDLE, or "
                   "choose a correlation valid for PIPES");
      }
      else if (flow_channel.getHeatTransferGeometry() ==
               FlowChannelBase::EConvHeatTransGeom::ROD_BUNDLE)
      {
        params.set<MooseEnum>("bundle_array") = "SQUARE";
      }
      else if (flow_channel.getHeatTransferGeometry() ==
               FlowChannelBase::EConvHeatTransGeom::HEX_ROD_BUNDLE)
      {
        params.set<MooseEnum>("bundle_array") = "TRIANGULAR";
      }
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    case WallHTCClosureType::LYON:
    {
      const std::string class_name = "ADWallHeatTransferCoefficientLyonMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    case WallHTCClosureType::KAZIMI_CARELLI:
    {

      const std::string class_name = "ADWallHeatTransferCoefficientKazimiMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<Real>("PoD") = flow_channel.getParam<Real>("PoD");
      if (flow_channel.getParam<Real>("PoD") == 1.0)
      {
        mooseDoOnce(mooseWarning(
            "You are using a rod bundle correlation with the default Pitch-to-Diameter "
            "ratio value, P/D=1.0. It can be set using the PoD parameter in the corresponding "
            "FlowChannel1Phase component"));
      }
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    case WallHTCClosureType::MIKITYUK:
    {

      const std::string class_name = "ADWallHeatTransferCoefficientMikityukMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<Real>("PoD") = flow_channel.getParam<Real>("PoD");
      if (flow_channel.getParam<Real>("PoD") == 1.0)
      {
        mooseDoOnce(mooseWarning(
            "You are using a rod bundle correlation with the default Pitch-to-Diameter "
            "ratio value, P/D=1.0. It can be set using the PoD parameter in the corresponding "
            "FlowChannel1Phase component"));
      }
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    case WallHTCClosureType::SCHAD:
    {

      const std::string class_name = "ADWallHeatTransferCoefficientSchadMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      params.set<Real>("PoD") = flow_channel.getParam<Real>("PoD");
      if (flow_channel.getParam<Real>("PoD") == 1.0)
      {
        mooseDoOnce(mooseWarning(
            "You are using a rod bundle correlation with the default Pitch-to-Diameter "
            "ratio value, P/D=1.0. It can be set using the PoD parameter in the corresponding "
            "FlowChannel1Phase component"));
      }
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    case WallHTCClosureType::GNIELINSKI:
    {

      const std::string class_name = "ADWallHeatTransferCoefficientGnielinskiMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("Hw") = flow_channel.getWallHTCNames1Phase()[i];
      params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
      params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
      _sim.addMaterial(class_name, genName(flow_channel.name(), "whtc_mat", i), params);
      break;
    }
    default:
      mooseError("Invalid WallHTCClosureType");
  }
}
void
Closures1PhaseTHM::addTemperatureWallFromHeatFluxMaterial(const FlowChannel1Phase & flow_channel,
                                                          unsigned int i) const
{
  const std::string class_name = "TemperatureWallFromHeatFlux3EqnTHMMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("T_wall") = flow_channel.getWallTemperatureNames()[i];
  params.set<MaterialPropertyName>("D_h") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
  params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
  params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
  params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
  params.set<MaterialPropertyName>("k") = FlowModelSinglePhase::THERMAL_CONDUCTIVITY;
  params.set<MaterialPropertyName>("mu") = FlowModelSinglePhase::DYNAMIC_VISCOSITY;
  params.set<MaterialPropertyName>("cp") = FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE;
  params.set<MaterialPropertyName>("q_wall") = flow_channel.getWallHeatFluxNames()[i];
  _sim.addMaterial(class_name, genName(flow_channel.name(), "T_from_q_wall_mat", i), params);
}
