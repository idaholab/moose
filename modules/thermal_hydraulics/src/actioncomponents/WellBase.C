//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WellBase.h"
#include "PhysicalConstants.h"

InputParameters
WellBase::validParams()
{
  InputParameters params = THMActionComponent::validParams();

  params.addRequiredParam<Point>("surface_point", "Surface point [m]");
  params.addRequiredParam<std::vector<Point>>("junction_points", "Junction points [m]");
  params.addParam<Point>(
      "end_point", "End point [m]. If not provided, the well ends at the final volume junction");
  params.addRequiredParam<std::vector<unsigned int>>("section_n_elems",
                                                     "Number of elements in each well section");
  params.addRequiredParam<FunctionName>("area",
                                        "Cross-sectional flow area function of the well [m^2]");
  params.addRequiredParam<std::vector<Real>>("junction_coupling_areas",
                                             "Flow surface area for each junction point [m^2]");
  params.addRequiredParam<RealVectorValue>("fracture_direction", "Direction to fracture");
  params.addRequiredParam<Real>("junction_volume", "Volume of each junction [m^3]");

  params.addRequiredParam<FunctionName>("initial_pressure", "Initial pressure function [Pa]");
  params.addRequiredParam<FunctionName>("initial_temperature", "Initial temperature function [K]");

  params.addRequiredParam<UserObjectName>("fluid_properties", "Fluid properties object");
  params.addParam<FunctionName>("friction_factor", "0.0", "Darcy friction factor function");

  params.addParam<MultiAppName>(
      "multi_app",
      "If provided, transfers occur with this MultiApp. The following would be transferred: mass "
      "flow rate, energy flow rate, temperature, and pressure at each junction.");

  return params;
}

WellBase::WellBase(const InputParameters & params)
  : THMActionComponent(params),
    _surface_point(getParam<Point>("surface_point")),
    _junction_points(getParam<std::vector<Point>>("junction_points")),
    _section_n_elems(getParam<std::vector<unsigned int>>("section_n_elems")),
    _n_sections(_section_n_elems.size()),
    _junction_coupling_areas(getParam<std::vector<Real>>("junction_coupling_areas")),
    _closures_name(name() + "_closures")
{
  _all_points.push_back(_surface_point);
  for (const auto i : index_range(_junction_points))
    _all_points.push_back(_junction_points[i]);
  if (isParamValid("end_point"))
    _all_points.push_back(getParam<Point>("end_point"));

  checkVectorParamsSameLength<Point, unsigned int>("junction_points", "section_n_elems");
  checkVectorParamsSameLength<Point, Real>("junction_points", "junction_coupling_areas");
}

void
WellBase::addClosures()
{
  const std::string class_name = "Closures1PhaseSimple";
  auto params = _factory.getValidParams(class_name);
  addClosuresObject(class_name, _closures_name, params);
}

void
WellBase::addWellBaseComponents(bool is_production)
{
  for (const auto i : index_range(_all_points))
    if (i != 0)
      addFlowChannel(i - 1, is_production);

  if (isParamValid("end_point"))
    addWall(is_production);

  for (const auto i : index_range(_junction_points))
  {
    addJunction(i, is_production);
    addJunctionFlux(i);
  }
}

void
WellBase::addFlowChannel(unsigned int i, bool is_production)
{
  Point start_position, end_position;
  if (is_production)
  {
    start_position = _all_points[i + 1];
    end_position = _all_points[i];
  }
  else
  {
    start_position = _all_points[i];
    end_position = _all_points[i + 1];
  }

  const RealVectorValue translation = end_position - start_position;

  const std::string class_name = "FlowChannel1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<Point>("position") = start_position;
  params.set<RealVectorValue>("orientation") = translation;
  params.set<std::vector<Real>>("length") = {translation.norm()};
  params.set<std::vector<unsigned int>>("n_elems") = {_section_n_elems[i]};
  params.set<FunctionName>("A") = getParam<FunctionName>("area");
  params.set<FunctionName>("initial_p") = getParam<FunctionName>("initial_pressure");
  params.set<FunctionName>("initial_T") = getParam<FunctionName>("initial_temperature");
  params.set<FunctionName>("initial_vel") = "0.0";
  params.set<RealVectorValue>("gravity_vector") = {
      0, 0, -PhysicalConstants::acceleration_of_gravity};
  params.set<UserObjectName>("fp") = getParam<UserObjectName>("fluid_properties");
  params.set<std::vector<std::string>>("closures") = {_closures_name};
  params.set<FunctionName>("f") = getParam<FunctionName>("friction_factor");
  params.set<MooseEnum>("rdg_slope_reconstruction") = "full";
  params.set<std::vector<Real>>("scaling_factor_1phase") = {1.0, 1.0, 1e-5};
  params.set<std::vector<VariableName>>("vpp_vars") = {"p"};
  params.set<bool>("create_flux_vpp") = true;
  addTHMComponent(class_name, flowChannelName(i), params);
}

void
WellBase::addWall(bool is_production)
{
  const std::string in_or_out = is_production ? ":in" : ":out";

  const std::string class_name = "SolidWall1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<BoundaryName>("input") = flowChannelName(_n_sections - 1) + in_or_out;
  addTHMComponent(class_name, name() + "_wall", params);
}

void
WellBase::addJunction(unsigned int i, bool is_production)
{
  const std::string first_end = is_production ? ":in" : ":out";
  const std::string second_end = is_production ? ":out" : ":in";
  std::vector<BoundaryName> connections;
  connections.push_back(flowChannelName(i) + first_end);
  if (i + 1 <= _n_sections - 1)
    connections.push_back(flowChannelName(i + 1) + second_end);

  const std::string class_name = "VolumeJunction1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<Point>("position") = _junction_points[i];
  params.set<std::vector<BoundaryName>>("connections") = connections;
  params.set<FunctionName>("initial_p") = getParam<FunctionName>("initial_pressure");
  params.set<FunctionName>("initial_T") = getParam<FunctionName>("initial_temperature");
  params.set<FunctionName>("initial_vel_x") = "0.0";
  params.set<FunctionName>("initial_vel_y") = "0.0";
  params.set<FunctionName>("initial_vel_z") = "0.0";
  params.set<Real>("volume") = getParam<Real>("junction_volume");
  params.set<Real>("scaling_factor_rhoEV") = 1e-5;
  addTHMComponent(class_name, volumeJunctionName(i), params);
}

void
WellBase::addJunctionFlux(unsigned int i)
{
  const std::string class_name = "VolumeJunctionCoupledFlux1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<Real>("A_coupled") = _junction_coupling_areas[i];
  params.set<RealVectorValue>("normal_from_junction") =
      getParam<RealVectorValue>("fracture_direction");
  params.set<std::string>("volume_junction") = volumeJunctionName(i);
  params.set<std::string>("pp_suffix") = name() + std::to_string(i + 1);
  if (isParamValid("multi_app"))
    params.set<MultiAppName>("multi_app") = getParam<MultiAppName>("multi_app");
  addTHMComponent(class_name, volumeJunctionName(i) + "_flux", params);
}

std::string
WellBase::flowChannelName(unsigned int i) const
{
  return name() + std::to_string(i + 1);
}

std::string
WellBase::volumeJunctionName(unsigned int i) const
{
  return name() + "_junc" + std::to_string(i + 1);
}
