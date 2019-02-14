#include "PipeBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseNCGFluidProperties.h"

const std::map<std::string, PipeBase::EConvHeatTransGeom> PipeBase::_heat_transfer_geom_to_enum{
    {"PIPE", PIPE}, {"ROD_BUNDLE", ROD_BUNDLE}};

MooseEnum
PipeBase::getConvHeatTransGeometry(const std::string & name)
{
  return THM::getMooseEnum<EConvHeatTransGeom>(name, _heat_transfer_geom_to_enum);
}

template <>
PipeBase::EConvHeatTransGeom
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<PipeBase::EConvHeatTransGeom>(s, PipeBase::_heat_transfer_geom_to_enum);
}

template <>
InputParameters
validParams<PipeBase>()
{
  InputParameters params = validParams<GeometricalFlowComponent>();
  params.addRequiredParam<FunctionName>("A", "Area of the pipe, can be a constant or a function");
  params.addPrivateParam<std::string>("component_type", "pipe");
  return params;
}

PipeBase::PipeBase(const InputParameters & params)
  : GeometricalFlowComponent(params),
    _flow_model(nullptr),
    _area_function(getParam<FunctionName>("A"))
{
}

std::shared_ptr<const FlowModel>
PipeBase::getFlowModel() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _flow_model;
}

std::shared_ptr<FlowModel>
PipeBase::buildFlowModel()
{
  const std::string class_name = _app.getFlowModelClassName(_model_id);
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<PipeBase *>("_pipe") = this;
  pars.set<UserObjectName>("fp") = _fp_name;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<AuxVariableName>("A_linear_name") = _A_linear_name;
  pars.set<MooseEnum>("rdg_slope_reconstruction") = _rdg_slope_reconstruction;
  if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
    pars.set<UserObjectName>("rdg_int_var_uo_name") = _rdg_int_var_uo_name;
  return _factory.create<FlowModel>(class_name, name(), pars, 0);
}

void
PipeBase::init()
{
  GeometricalFlowComponent::init();

  _flow_model = buildFlowModel();
  _flow_model->init();
}
