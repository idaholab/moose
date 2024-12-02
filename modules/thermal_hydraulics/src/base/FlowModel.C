//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMProblem.h"
#include "Component.h"
#include "FlowChannelBase.h"
#include "ConstantFunction.h"
#include "THMNames.h"

using namespace libMesh;

InputParameters
FlowModel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.addPrivateParam<FlowChannelBase *>("_flow_channel");
  params.addRequiredParam<UserObjectName>(
      "fp", "The name of the user object that defines fluid properties");
  params.addRequiredParam<bool>("output_vector_velocity",
                                "True if velocity is put out as a vector field.");
  params.registerBase("THM:flow_model");
  return params;
}

const std::string FlowModel::AREA = THM::AREA;
const std::string FlowModel::AREA_LINEAR = THM::AREA_LINEAR;
const std::string FlowModel::HEAT_FLUX_WALL = THM::HEAT_FLUX_WALL;
const std::string FlowModel::HEAT_FLUX_PERIMETER = THM::HEAT_FLUX_PERIMETER;
const std::string FlowModel::NUSSELT_NUMBER = THM::NUSSELT_NUMBER;
const std::string FlowModel::TEMPERATURE_WALL = THM::TEMPERATURE_WALL;
const std::string FlowModel::UNITY = THM::UNITY;
const std::string FlowModel::DIRECTION = THM::DIRECTION;

FlowModel::FlowModel(const InputParameters & params)
  : MooseObject(params),
    _sim(*params.getCheckedPointerParam<THMProblem *>("_thm_problem")),
    _factory(_app.getFactory()),
    _flow_channel(*params.getCheckedPointerParam<FlowChannelBase *>("_flow_channel")),
    _fe_type(_sim.getFlowFEType()),
    _fp_name(params.get<UserObjectName>("fp")),
    _comp_name(name()),
    _gravity_vector(_flow_channel.getParam<RealVectorValue>("gravity_vector")),
    _gravity_magnitude(_gravity_vector.norm()),
    _lump_mass_matrix(_flow_channel.getParam<bool>("lump_mass_matrix")),
    _output_vector_velocity(params.get<bool>("output_vector_velocity"))
{
}

const FunctionName &
FlowModel::getVariableFn(const FunctionName & fn_param_name)
{
  const FunctionName & fn_name = _flow_channel.getParam<FunctionName>(fn_param_name);
  const Function & fn = _sim.getFunction(fn_name);

  if (dynamic_cast<const ConstantFunction *>(&fn) != nullptr)
  {
    _flow_channel.connectObject(fn.parameters(), fn_name, fn_param_name, "value");
  }

  return fn_name;
}

void
FlowModel::addCommonVariables()
{
  const std::vector<SubdomainName> & subdomains = _flow_channel.getSubdomainNames();

  _sim.addSimVariable(false, AREA, _fe_type, subdomains);
  _sim.addSimVariable(false, HEAT_FLUX_PERIMETER, _fe_type, subdomains);
  _sim.addSimVariable(false, AREA_LINEAR, FEType(FIRST, LAGRANGE), subdomains);
}

void
FlowModel::addCommonInitialConditions()
{
  if (_flow_channel.isParamValid("A") && !_app.isRestarting())
  {
    const std::vector<SubdomainName> & block = _flow_channel.getSubdomainNames();
    const FunctionName & area_function = _flow_channel.getAreaFunctionName();

    if (!_sim.hasFunction(area_function))
    {
      const Function & fn = _sim.getFunction(area_function);
      _sim.addConstantIC(AREA, fn.value(0, Point()), block);
      _sim.addConstantIC(AREA_LINEAR, fn.value(0, Point()), block);

      _flow_channel.makeFunctionControllableIfConstant(area_function, "Area", "value");
    }
    else
    {
      _sim.addFunctionIC(AREA_LINEAR, area_function, block);

      {
        const std::string class_name = "FunctionNodalAverageIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = AREA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<FunctionName>("function") = area_function;
        _sim.addSimInitialCondition(class_name, genName(_comp_name, AREA, "ic"), params);
      }
    }
  }
}

void
FlowModel::addCommonMooseObjects()
{
  // add material property equal to one, useful for dummy multiplier values
  {
    const std::string class_name = "ConstantMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::string>("property_name") = FlowModel::UNITY;
    params.set<Real>("value") = 1.0;
    params.set<std::vector<VariableName>>("derivative_vars") = _derivative_vars;
    _sim.addMaterial(class_name, genName(_comp_name, FlowModel::UNITY), params);
  }
}
