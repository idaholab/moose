#include "PipeBase.h"
#include "FluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

const std::string PipeBase::_type("pipe");

template <>
InputParameters
validParams<PipeBase>()
{
  InputParameters params = validParams<GeometricalComponent>();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid property user object");
  params.addPrivateParam<std::string>("component_type", PipeBase::_type);
  return params;
}

PipeBase::PipeBase(const InputParameters & params)
  : GeometricalComponent(params), _fp_name(getParam<UserObjectName>("fp")), _flow_model(NULL)
{
}

UserObjectName
PipeBase::getFluidPropertiesName() const
{
  return _fp_name;
}

void
PipeBase::init()
{
  _model_id = _app.getFlowModelID(_sim.getUserObject<FluidProperties>(_fp_name));
  if (_model_id == RELAP7::FM_SINGLE_PHASE)
  {
    _flow_model = new FlowModelSinglePhase(genName(name(), "flow_model"), parameters());
    _flow_model->init();
  }
  else if (_model_id == RELAP7::FM_TWO_PHASE)
  {
    _flow_model = new FlowModelTwoPhase(genName(name(), "flow_model"), parameters());
    _flow_model->init();
  }
  else
    logError("Unknown model type supplied, '", _model_id, "'");
}
