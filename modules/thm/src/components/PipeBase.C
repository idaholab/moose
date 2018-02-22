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
  : GeometricalComponent(params), _fp_name(getParam<UserObjectName>("fp")), _flow_model(nullptr)
{
}

std::shared_ptr<const FlowModel>
PipeBase::getFlowModel() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _flow_model;
}

const RELAP7::FlowModelID &
PipeBase::getFlowModelID() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _model_id;
}

void
PipeBase::init()
{
  GeometricalComponent::init();

  _model_id = _app.getFlowModelID(_sim.getUserObject<FluidProperties>(_fp_name));
  if (_model_id == RELAP7::FM_SINGLE_PHASE)
  {
    InputParameters pars = emptyInputParameters();
    pars.set<Simulation *>("_sim") = &_sim;
    pars.set<PipeBase *>("_pipe") = this;
    pars.set<bool>("2nd_order_mesh") = _2nd_order_mesh;
    _flow_model = std::make_shared<FlowModelSinglePhase>(name(), pars);
    _flow_model->init();
  }
  else if (_model_id == RELAP7::FM_TWO_PHASE)
  {
    InputParameters pars = emptyInputParameters();
    pars.set<Simulation *>("_sim") = &_sim;
    pars.set<PipeBase *>("_pipe") = this;
    pars.set<bool>("2nd_order_mesh") = _2nd_order_mesh;
    _flow_model = std::make_shared<FlowModelTwoPhase>(name(), pars);
    _flow_model->init();
  }
  else
    logModelNotImplementedError(_model_id);
}

void
PipeBase::check()
{
  GeometricalComponent::check();
}
