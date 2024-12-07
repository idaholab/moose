//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionModel.h"
#include "THMProblem.h"
#include "Factory.h"
#include "Component.h"
#include "ThermalHydraulicsApp.h"
#include "HeatStructureInterface.h"
#include "HeatStructureCylindricalBase.h"

using namespace libMesh;

InputParameters
HeatConductionModel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.addPrivateParam<HeatStructureInterface *>("_hs");
  params.addRequiredParam<Real>("scaling_factor_temperature",
                                "Scaling factor for solid temperature variable.");
  params.registerBase("THM:heat_conduction_model");
  return params;
}

registerMooseObject("ThermalHydraulicsApp", HeatConductionModel);

const std::string HeatConductionModel::DENSITY = "density";
const std::string HeatConductionModel::TEMPERATURE = "T_solid";
const std::string HeatConductionModel::THERMAL_CONDUCTIVITY = "thermal_conductivity";
const std::string HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE = "specific_heat";

FEType HeatConductionModel::_fe_type(FIRST, LAGRANGE);

HeatConductionModel::HeatConductionModel(const InputParameters & params)
  : MooseObject(params),
    _sim(*params.getCheckedPointerParam<THMProblem *>("_thm_problem")),
    _factory(_app.getFactory()),
    _hs_interface(*params.getCheckedPointerParam<HeatStructureInterface *>("_hs")),
    _geometrical_component(
        dynamic_cast<GeometricalComponent &>(_hs_interface)), // TODO: do something safer
    _comp_name(name())
{
}

void
HeatConductionModel::addVariables()
{
  const auto & subdomain_names = _geometrical_component.getSubdomainNames();
  const Real & scaling_factor = getParam<Real>("scaling_factor_temperature");

  _sim.addSimVariable(true, TEMPERATURE, _fe_type, subdomain_names, scaling_factor);
}

void
HeatConductionModel::addInitialConditions()
{
  const auto & subdomain_names = _geometrical_component.getSubdomainNames();
  _sim.addFunctionIC(TEMPERATURE, _hs_interface.getInitialT(), subdomain_names);
}

void
HeatConductionModel::addMaterials()
{
  const auto & blocks = _geometrical_component.getSubdomainNames();
  const auto & material_names =
      _geometrical_component.getParam<std::vector<std::string>>("materials");

  for (std::size_t i = 0; i < blocks.size(); i++)
  {
    std::string class_name = "ADSolidMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = {blocks[i]};
    params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
    params.set<UserObjectName>("properties") = material_names[i];
    _sim.addMaterial(class_name, genName(blocks[i], "mat"), params);
  }
}

void
HeatConductionModel::addHeatEquationXYZ()
{
  const auto & blocks = _geometrical_component.getSubdomainNames();

  // add transient term
  if (_geometrical_component.problemIsTransient())
  {
    std::string class_name = "ADHeatConductionTimeDerivative";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
    pars.set<MaterialPropertyName>("density_name") = DENSITY;
    pars.set<bool>("use_displaced_mesh") = false;
    _sim.addKernel(class_name, genName(_comp_name, "td"), pars);
  }
  // add diffusion term
  {
    std::string class_name = "ADHeatConduction";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("thermal_conductivity") = THERMAL_CONDUCTIVITY;
    pars.set<bool>("use_displaced_mesh") = false;
    _sim.addKernel(class_name, genName(_comp_name, "hc"), pars);
  }
}

void
HeatConductionModel::addHeatEquationRZ()
{
  HeatStructureCylindricalBase & hs_cyl =
      dynamic_cast<HeatStructureCylindricalBase &>(_geometrical_component);

  const auto & blocks = hs_cyl.getSubdomainNames();
  const auto & position = hs_cyl.getPosition();
  const auto & direction = hs_cyl.getDirection();

  // add transient term
  if (_geometrical_component.problemIsTransient())
  {
    std::string class_name = "ADHeatConductionTimeDerivativeRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
    pars.set<MaterialPropertyName>("density_name") = DENSITY;
    pars.set<bool>("use_displaced_mesh") = false;
    pars.set<Point>("axis_point") = position;
    pars.set<RealVectorValue>("axis_dir") = direction;
    _sim.addKernel(class_name, genName(_comp_name, "td"), pars);
  }
  // add diffusion term
  {
    std::string class_name = "ADHeatConductionRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("thermal_conductivity") = THERMAL_CONDUCTIVITY;
    pars.set<bool>("use_displaced_mesh") = false;
    pars.set<Point>("axis_point") = position;
    pars.set<RealVectorValue>("axis_dir") = direction;
    _sim.addKernel(class_name, genName(_comp_name, "hc"), pars);
  }
}
