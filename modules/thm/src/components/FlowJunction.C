#include "FlowJunction.h"
#include "Factory.h"
#include "Conversion.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "GeometricalComponent.h"
#include "FluidPropertiesBase.h"
#include <sstream>


template<>
InputParameters validParams<FlowJunction>()
{
  InputParameters params = validParams<Junction>();
  params.addParam<std::vector<Real> >("K", "Form loss coefficients");
  params.addParam<Real>("scaling_factor", 1., "Scaling factor for the Lagrange multiplier variable");
  std::vector<Real> sf4(4, 1.);
  sf4[0] = 1.e+3;
  sf4[1] = 1.e+0;
  sf4[2] = 1.e+6;
  sf4[3] = 1.;
  params.addParam<std::vector<Real> >("scaling_factor_bcs", sf4, "Scaling factors for the BCs");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  return params;
}


FlowJunction::FlowJunction(const InputParameters & params) :
    Junction(params),
    _lm_name(genName(name(), "lm")),
    _K(getParam<std::vector<Real> >("K")),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _scaling_factor_bcs(getParam<std::vector<Real> >("scaling_factor_bcs"))
{
}

FlowJunction::~FlowJunction()
{
}

void
FlowJunction::init()
{
  const FluidPropertiesBase & fp = _sim.getUserObject<FluidPropertiesBase>(getParam<UserObjectName>("fp"));
  _model_type = fp.modelType();
}

void
FlowJunction::addVariables()
{
  std::vector<unsigned int> connected_subdomains;
  this->getConnectedSubdomains(connected_subdomains);

  // add scalar variable (i.e. Lagrange multiplier)
  switch (_model_type)
  {
  case FlowModel::EQ_MODEL_2:
    _sim.addVariable(true, _lm_name,  FEType(SECOND, SCALAR), connected_subdomains, _scaling_factor);
    break;

  case FlowModel::EQ_MODEL_3:
    _sim.addVariable(true, _lm_name,  FEType(THIRD, SCALAR), connected_subdomains, _scaling_factor);
    break;

  default:
    mooseError(name() << ": Not implemented yet.");
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
  std::vector<VariableName> cv_rhoA(1, FlowModel::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModel::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModel::RHOEA);
  std::vector<VariableName> cv_v(1, FlowModel::SPECIFIC_VOLUME);
  std::vector<VariableName> cv_e(1, FlowModel::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_lambda(1, _lm_name);

  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "linear";

  std::string c_pps = genName(name(), "flow_pps");
  {
    InputParameters params = _factory.getValidParams("PressureCB");
    params.set<std::vector<unsigned int> >("r7:boundary") = _bnd_id;
    params.set<MultiMooseEnum>("execute_on") = execute_options;
    params.set<std::string>("r7:output") = "none";
    // coupling
    params.set<std::vector<VariableName> >("area") = cv_area;

    _sim.addPostprocessor("PressureCB", c_pps, params);
  }

  // add BC terms
  for (unsigned int i = 0; i < _bnd_id.size(); ++i)
  {
    std::vector<unsigned int> bnd_id(1, _bnd_id[i]);

    // mass
    {
      InputParameters params = _factory.getValidParams("OneDFreeMassBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHOA;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = _scaling_factor_bcs;
      // coupling
      params.set<std::vector<VariableName> >("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName> >("rhouA") = cv_rhouA;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      if (_model_type == FlowModel::EQ_MODEL_3)
        params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;

      _sim.addBoundaryCondition("OneDFreeMassBC", genName(name(), _bnd_id[i], "mass_bc"), params);
    }
    // momentum
    {
      InputParameters params = _factory.getValidParams("OneDFreeMomentumBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHOUA;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = _scaling_factor_bcs;
      // coupling
      params.set<std::vector<VariableName> >("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("pressure") = cv_pressure;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      if (_model_type == FlowModel::EQ_MODEL_3)
      {
        params.set<std::vector<VariableName> >("rhoEA") = cv_rhoEA;
        params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
      }

      _sim.addBoundaryCondition("OneDFreeMomentumBC", genName(name(), _bnd_id[i], "mom_bc"), params);
    }
    // energy
    if (_model_type == FlowModel::EQ_MODEL_3)
    {
      InputParameters params = _factory.getValidParams("OneDFreeEnergyBC");
      params.set<NonlinearVariableName>("variable") = FlowModel::RHOEA;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      // coupling
      params.set<std::vector<VariableName> >("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName> >("rhouA") = cv_rhouA;
      params.set<std::vector<VariableName> >("u") = cv_u;
      params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
      params.set<std::vector<VariableName> >("area") = cv_area;
      params.set<PostprocessorName>("c") = c_pps;
      params.set<std::vector<Real> >("scaling_factors") = _scaling_factor_bcs;
      params.set<std::vector<VariableName> >("lambda") = cv_lambda;

      _sim.addBoundaryCondition("OneDFreeEnergyBC", genName(name(), _bnd_id[i], "erg_bc"), params);
    }
  }

  // add the constraints
  {
    InputParameters params = _factory.getValidParams("FlowConstraint");
    params.set<NonlinearVariableName>("variable") = _lm_name;

    params.set<FlowModel::EModelType>("model_type") = _model_type;
    params.set<std::vector<dof_id_type> >("nodes") = _nodes;
    params.set<std::vector<Real> >("normals") = _normals;
    params.set<std::vector<Real> >("K") = _K;
    params.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
    // coupling
    params.set<std::vector<VariableName> >("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName> >("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName> >("e") = cv_e;
    params.set<std::vector<VariableName> >("v") = cv_v;
    params.set<std::vector<VariableName> >("u") = cv_u;
    params.set<std::vector<VariableName> >("p") = cv_pressure;
    params.set<std::vector<VariableName> >("area") = cv_area;
    if (_model_type == FlowModel::EQ_MODEL_3)
    {
      params.set<std::vector<VariableName> >("rhoEA") = cv_rhoEA;
      params.set<std::vector<VariableName> >("enthalpy") = cv_enthalpy;
    }

    _sim.addScalarKernel("FlowConstraint", genName(name(), "flow_c0"), params);
  }
}
