#include "PipeBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

template <>
InputParameters
validParams<PipeBase>()
{
  InputParameters params = validParams<GeometricalFlowComponent>();
  params.addPrivateParam<std::string>("component_type", "pipe");
  return params;
}

PipeBase::PipeBase(const InputParameters & params)
  : GeometricalFlowComponent(params), _flow_model(nullptr)
{
}

std::shared_ptr<const FlowModel>
PipeBase::getFlowModel() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _flow_model;
}

void
PipeBase::init()
{
  GeometricalFlowComponent::init();

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
