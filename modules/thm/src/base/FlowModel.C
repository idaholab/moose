#include "Simulation.h"
#include "Component.h"
#include "FlowChannelBase.h"
#include "ConstantFunction.h"

template <>
InputParameters
validParams<FlowModel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<Simulation *>("_sim");
  params.addPrivateParam<FlowChannelBase *>("_flow_channel");
  params.addRequiredParam<UserObjectName>(
      "fp", "The name of the user object that defines fluid properties");
  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");
  params.addRequiredParam<AuxVariableName>("A_linear_name",
                                           "Linear cross-sectional area variable name");
  params.addRequiredParam<MooseEnum>("rdg_slope_reconstruction",
                                     "Slope reconstruction type for rDG");
  params.registerBase("THM:flow_model");
  return params;
}

const std::map<std::string, FlowModel::EEquationType> FlowModel::_flow_equation_type_to_enum{
    {"CONTINUITY", CONTINUITY},
    {"MOMENTUM", MOMENTUM},
    {"ENERGY", ENERGY},
    {"VOIDFRACTION", VOIDFRACTION}};

MooseEnum
FlowModel::getFlowEquationType(const std::string & name)
{
  return THM::getMooseEnum<EEquationType>(name, _flow_equation_type_to_enum);
}

template <>
FlowModel::EEquationType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowModel::EEquationType>(s, FlowModel::_flow_equation_type_to_enum);
}

const std::map<std::string, FlowModel::ESpatialDiscretizationType>
    FlowModel::_spatial_disc_type_to_enum{{"CG", CG}, {"RDG", rDG}};

MooseEnum
FlowModel::getSpatialDiscretizationMooseEnum(const std::string & name)
{
  return THM::getMooseEnum<ESpatialDiscretizationType>(name, _spatial_disc_type_to_enum);
}

template <>
FlowModel::ESpatialDiscretizationType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowModel::ESpatialDiscretizationType>(s,
                                                             FlowModel::_spatial_disc_type_to_enum);
}

const std::string FlowModel::AREA = "A";
const std::string FlowModel::HEAT_FLUX_WALL = "q_wall";
const std::string FlowModel::HEAT_FLUX_PERIMETER = "P_hf";
const std::string FlowModel::HYDRAULIC_DIAMETER = "D_h";
const std::string FlowModel::NUSSELT_NUMBER = "Nu";
const std::string FlowModel::TEMPERATURE_WALL = "T_wall";
const std::string FlowModel::UNITY = "unity";
const std::string FlowModel::DIRECTION = "direction";

FEType FlowModel::_fe_type = FEType(CONSTANT, MONOMIAL);
FlowModel::ESpatialDiscretizationType FlowModel::_spatial_discretization = FlowModel::rDG;

FlowModel::FlowModel(const InputParameters & params)
  : MooseObject(params),
    _sim(*params.getCheckedPointerParam<Simulation *>("_sim")),
    _app(_sim.getApp()),
    _factory(_app.getFactory()),
    _flow_channel(*params.getCheckedPointerParam<FlowChannelBase *>("_flow_channel")),
    _fp_name(params.get<UserObjectName>("fp")),
    _comp_name(name()),
    _gravity_vector(_flow_channel.getParam<RealVectorValue>("gravity_vector")),
    _gravity_magnitude(_gravity_vector.norm()),
    _lump_mass_matrix(_flow_channel.getParam<bool>("lump_mass_matrix")),
    _A_linear_name(params.get<AuxVariableName>("A_linear_name")),
    _rdg_slope_reconstruction(params.get<MooseEnum>("rdg_slope_reconstruction")),
    _numerical_flux_name(params.get<UserObjectName>("numerical_flux"))
{
}

const FunctionName &
FlowModel::getVariableFn(const FunctionName & fn_param_name)
{
  const FunctionName & fn_name = _flow_channel.getParam<FunctionName>(fn_param_name);
  const Function & fn = _sim.getFunction(fn_name);

  if (dynamic_cast<const ConstantFunction *>(&fn) != nullptr)
  {
    _flow_channel.connectObject(fn.parameters(), "", fn_name, fn_param_name, "value");
  }

  return fn_name;
}

void
FlowModel::addCommonVariables()
{
  unsigned int subdomain_id = _flow_channel.getSubdomainID();

  _sim.addVariable(false, AREA, _fe_type, subdomain_id);
  _sim.addVariable(false, HEAT_FLUX_PERIMETER, _fe_type, subdomain_id);

  if (_spatial_discretization == rDG)
    _sim.addVariable(false, _A_linear_name, FEType(FIRST, LAGRANGE), subdomain_id);
}

void
FlowModel::addCommonInitialConditions()
{
  if (_flow_channel.isParamValid("A"))
  {
    const std::vector<SubdomainName> & block = _flow_channel.getSubdomainNames();
    const FunctionName & area_function = _flow_channel.getAreaFunctionName();

    if (!_sim.hasFunction(area_function))
    {
      Function & fn = _sim.getFunction(area_function);
      _sim.addConstantIC(AREA, fn.value(0, Point()), block);
      if (getSpatialDiscretizationType() == rDG)
        _sim.addConstantIC(_A_linear_name, fn.value(0, Point()), block);
      // FIXME: eventually use Component::makeFunctionControllableIfConstant
      if (dynamic_cast<ConstantFunction *>(&fn) != nullptr)
        _flow_channel.connectObject(fn.parameters(), "", area_function, "Area", "value");
    }
    else
    {
      if (getSpatialDiscretizationType() == rDG)
      {
        _sim.addFunctionIC(_A_linear_name, area_function, block);

        {
          const std::string class_name = "FunctionNodalAverageIC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = AREA;
          params.set<std::vector<SubdomainName>>("block") = block;
          params.set<FunctionName>("function") = area_function;
          _sim.addInitialCondition(class_name, Component::genName(_comp_name, AREA, "ic"), params);
        }
      }
      else
        _sim.addFunctionIC(AREA, area_function, block);
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
    _sim.addMaterial(class_name, Component::genName(_comp_name, FlowModel::UNITY), params);
  }
}
