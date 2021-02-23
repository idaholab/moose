#include "SupersonicInlet.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", SupersonicInlet);

InputParameters
SupersonicInlet::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addParam<Real>("p", "Prescribed pressure [Pa]");
  params.addParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<Real>("vel", "Prescribed velocity [m/s]");
  return params;
}

SupersonicInlet::SupersonicInlet(const InputParameters & parameters)
  : FlowBoundary1Phase(parameters)
{
}

void
SupersonicInlet::check() const
{
  FlowBoundary1Phase::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);

  if (_spatial_discretization == FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
SupersonicInlet::addMooseObjects()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  const SinglePhaseFluidProperties & spfp =
      _sim.getUserObject<SinglePhaseFluidProperties>(_fp_name);

  Real p = getParam<Real>("p");
  Real T = getParam<Real>("T");
  Real vel = getParam<Real>("vel");

  Real rho = spfp.rho_from_p_T(p, T);
  Real e = spfp.e_from_p_rho(p, rho);
  Real rhou = rho * vel;
  Real rhoE = rho * (e + 0.5 * vel * vel);
  {
    std::string class_name = "OneDAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("value") = rho;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoA_bc"), params);
  }
  {
    std::string class_name = "OneDAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("value") = rhou;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addBoundaryCondition(class_name, genName(name(), "rhouA_bc"), params);
  }
  {
    std::string class_name = "OneDAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("value") = rhoE;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoEA_bc"), params);
  }
}
