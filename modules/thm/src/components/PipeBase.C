#include "PipeBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseNCGFluidProperties.h"

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

  // create and initialize flow model
  InputParameters pars = emptyInputParameters();
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<PipeBase *>("_pipe") = this;
  pars.set<bool>("2nd_order_mesh") = _2nd_order_mesh;
  pars.set<UserObjectName>("rdg_flux") = _rdg_flux_name;
  pars.set<bool>("implicit_rdg") = _implicit_rdg;
  if (_model_id == RELAP7::FM_SINGLE_PHASE)
    _flow_model = std::make_shared<FlowModelSinglePhase>(name(), pars);
  else if (_model_id == RELAP7::FM_TWO_PHASE)
    _flow_model = std::make_shared<FlowModelTwoPhase>(name(), pars);
  else if (_model_id == RELAP7::FM_TWO_PHASE_NCG)
  {
    const TwoPhaseNCGFluidProperties & fp =
        _sim.getUserObject<TwoPhaseNCGFluidProperties>(_fp_name);
    pars.set<unsigned int>("n_ncgs") = fp.getNumberOfNCGs();
    _flow_model = std::make_shared<FlowModelTwoPhaseNCG>(name(), pars);
  }
  else
    logModelNotImplementedError(_model_id);
  _flow_model->init();
}
