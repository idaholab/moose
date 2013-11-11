#include "FlowJunction.h"
#include "Factory.h"
#include "Conversion.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "GeometricalComponent.h"
#include <sstream>


template<>
InputParameters validParams<FlowJunction>()
{
  InputParameters params = validParams<Junction>();
  params.addParam<std::vector<Real> >("K", "Form loss coefficients");
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  return params;
}


FlowJunction::FlowJunction(const std::string & name, InputParameters params) :
    Junction(name, params),
    _lm_name(genName(name, "lm")),
    _K(getParam<std::vector<Real> >("K"))
{
}

FlowJunction::~FlowJunction()
{
}

void
FlowJunction::addVariables()
{
  // add scalar variable (i.e. Lagrange multiplier)
  switch (_model_type)
  {
  case FlowModel::EQ_MODEL_2:
    _sim.addVariable(true, _lm_name,  FEType(SECOND, SCALAR), 0, 1.);
    break;

  case FlowModel::EQ_MODEL_3:
    _sim.addVariable(true, _lm_name,  FEType(THIRD, SCALAR), 0, 1.e-5);
    break;

  default:
    mooseError("Not implemented yet.");
    break;
  }
}

void
FlowJunction::addMooseObjects()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_u(1, FlowModel::VELOCITY);
  std::vector<VariableName> cv_pressure(1, FlowModel::PRESSURE);
  std::vector<VariableName> cv_enthalpy(1, FlowModel::ENTHALPY);
  std::vector<VariableName> cv_rho(1, FlowModel::RHO);
  std::vector<VariableName> cv_rhou(1, FlowModel::RHOU);
  std::vector<VariableName> cv_rhoE(1, FlowModel::RHOE);
  std::vector<VariableName> cv_lambda(1, _lm_name);
  std::vector<Real> sf = _sim.getParam<std::vector<Real> >("scaling_factors");

  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "residual";

  std::string c_pps = genName("flow", _id, "_pps");
  {
    InputParameters params = _factory.getValidParams("PressureCB");
    params.set<std::vector<unsigned int> >("r7:boundary") = _bnd_id;
    params.set<MooseEnum>("execute_on") = execute_options;
    params.set<MooseEnum>("output") = "none";
    params.set<UserObjectName>("eos")  = getParam<UserObjectName>("eos");
    // coupling
    params.set<std::vector<VariableName> >("area") = cv_area;
    params.set<std::vector<VariableName> >("pressure") = cv_pressure;

    _sim.addPostprocessor("PressureCB", c_pps, params);
  }

  // add BC terms
  for (unsigned int i = 0; i < _bnd_id.size(); ++i)
  {
    std::vector<unsigned int> bnd_id(1, _bnd_id[i]);

    // mass
    {
      InputParameters params = _factory.getValidParams("OneDFreeMassBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHO;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = sf;
      // coupling
      params.set<std::vector<VariableName> >("rho") = cv_rho;
      params.set<std::vector<VariableName> >("rhou") = cv_rhou;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      if (_model_type == FlowModel::EQ_MODEL_3)
      {
        params.set<std::vector<VariableName> >("rhoE") = cv_rhoE;
        params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
      }

      _sim.addBoundaryCondition("OneDFreeMassBC", genName("mass", _id, "_bc"), params);
    }
    // momentum
    {
      InputParameters params = _factory.getValidParams("OneDFreeMomentumBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHOU;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = sf;
      // coupling
      params.set<std::vector<VariableName> >("rho") = cv_rho;
      params.set<std::vector<VariableName> >("rhou") = cv_rhou;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("pressure") = cv_pressure;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      if (_model_type == FlowModel::EQ_MODEL_3)
      {
        params.set<std::vector<VariableName> >("rhoE") = cv_rhoE;
        params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
      }

      _sim.addBoundaryCondition("OneDFreeMomentumBC", genName("mom", _id, "_bc"), params);
    }
    // energy
    if (_model_type == FlowModel::EQ_MODEL_3)
    {
      InputParameters params = _factory.getValidParams("OneDFreeEnergyBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHOE;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      // coupling
      params.set<std::vector<VariableName> >("rho") = cv_rho;
      params.set<std::vector<VariableName> >("rhou") = cv_rhou;
      params.set<std::vector<VariableName> >("rhoE") = cv_rhoE;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = sf;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      _sim.addBoundaryCondition("OneDFreeEnergyBC", genName("erg", _id, "_bc"), params);
    }
  }

  // add the constraints
  {
    InputParameters params = _factory.getValidParams("FlowConstraint");
    params.set<NonlinearVariableName>("variable") = _lm_name;

    params.set<FlowModel::EModelType>("model_type") = _model_type;
    params.set<std::vector<unsigned int> >("nodes") = _nodes;
    params.set<std::vector<Real> >("normals") = _normals;
    params.set<std::vector<Real> >("K") = _K;
    params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
    // coupling
    params.set<std::vector<VariableName> >("u") = cv_u;
    params.set<std::vector<VariableName> >("p") = cv_pressure;
    params.set<std::vector<VariableName> >("rho") = cv_rho;
    params.set<std::vector<VariableName> >("rhou") = cv_rhou;
    params.set<std::vector<VariableName> >("area") = cv_area;
    if (_model_type == FlowModel::EQ_MODEL_3)
    {
      params.set<std::vector<VariableName> >("rhoE") = cv_rhoE;
      params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
    }

    _sim.addScalarKernel("FlowConstraint", genName("flow", _id, "_c0"), params);
  }
}
