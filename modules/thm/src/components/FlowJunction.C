#include "FlowJunction.h"
#include "Conversion.h"
#include "Simulation.h"
#include "Pipe.h"
#include "FEProblem.h"
#include "Factory.h"
#include <sstream>


template<>
InputParameters validParams<FlowJunction>()
{
  InputParameters params = validParams<Junction>();

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  params.addRequiredParam<Real>("junction_vol", "Volume of the junction");
  params.addRequiredParam<Real>("junction_gravity", "Gravity in the junction");
  params.addRequiredParam<Real>("junction_loss", "Loss in the junction");
  params.addRequiredParam<Real>("junction_area", "Area of the junction");

  // These params are not required, the junction will use global initial pressure, velocity,
  // and temperature values if these values are not specified.
  params.addParam<Real>("initial_P", "Initial pressure in the pipe");
  params.addParam<Real>("initial_V", "Initial velocity in the pipe");
  params.addParam<Real>("initial_T", "Initial temperature in the pipe");

  return params;
}


FlowJunction::FlowJunction(const std::string & name, InputParameters params) :
    Junction(name, params),
    _junction_rho_name(genName("junction_rho", _id, "")),
    _junction_rhou_name(genName("junction_rhou", _id, "")),
    _junction_rhoE_name(genName("junction_rhoE", _id, "")),
    _p_name(genName("lm", _id, "p")),
    _T_name(genName("lm", _id, "T")),
    _junction_vol(getParam<Real>("junction_vol")),
    _junction_gravity(getParam<Real>("junction_gravity")),
    _junction_loss(getParam<Real>("junction_loss")),
    _junction_area(getParam<Real>("junction_area")),
    _has_initial_P(params.isParamValid("initial_P")),
    _has_initial_V(params.isParamValid("initial_V")),
    _has_initial_T(params.isParamValid("initial_T")),
    _initial_P(_has_initial_P ? getParam<Real>("initial_P") : _sim.getParam<Real>("global_init_P")),
    _initial_V(_has_initial_V ? getParam<Real>("initial_V") : _sim.getParam<Real>("global_init_V")),
    _initial_T(_has_initial_T ? getParam<Real>("initial_T") : _sim.getParam<Real>("global_init_T"))
{
}

FlowJunction::~FlowJunction()
{
}


void
FlowJunction::addVariables()
{
  // add scalar variable (the conserved variables within the junction)
  switch (_model_type)
  {
  case FlowModel::EQ_MODEL_3:
  {
    // Set initial conditions for the junction variables.

    // Add three separate SCALAR junction variables with individual scaling factors
    _sim.addVariable(true, _junction_rho_name, FEType(FIRST,  SCALAR), 0, /*scale_factor=*/1.0);
    _sim.addVariable(true, _junction_rhou_name, FEType(FIRST,  SCALAR), 0, /*scale_factor=*/1.e-4);
    _sim.addVariable(true, _junction_rhoE_name, FEType(FIRST,  SCALAR), 0, /*scale_factor=*/1.e-6);

    // Compute initial (rho, rho*u, rho*E) in the FlowJunction using the initial (P,V,T) values
    // and the Simulation's EquationOfState object.
    Real
      initial_rho = 0.,
      initial_rhou = 0.,
      initial_rhoE = 0.;

    const EquationOfState & eos = _sim.getEquationOfState(getParam<UserObjectName>("eos"));

    initial_rho = eos.rho_from_p_T(_initial_P, _initial_T);
    initial_rhou = initial_rho * _initial_V;
    initial_rhoE = initial_rho * (eos.e_from_p_rho(_initial_P, initial_rho) + 0.5 * _initial_V * _initial_V);

    // Add initial conditions for these scalar values based on either the global initial (p,V,T)
    // values or the (p,V,T) values specified in the FlowJunction.
    _sim.addScalarInitialCondition(_junction_rho_name, initial_rho);
    _sim.addScalarInitialCondition(_junction_rhou_name, initial_rhou);
    _sim.addScalarInitialCondition(_junction_rhoE_name, initial_rhoE);

    // Add the SCALAR variables for temperature and pressure in the junction.
    _sim.addVariable(false, _p_name, FEType(FIRST,  SCALAR), /*subdomain_id=*/0, /*scale_factor=*/1.);
    _sim.addVariable(false, _T_name, FEType(FIRST,  SCALAR), /*subdomain_id=*/0, /*scale_factor=*/1.);

    // Add initial conditions for the SCALAR aux variables.
    _sim.addScalarInitialCondition(_p_name, _initial_P);
    _sim.addScalarInitialCondition(_T_name, _initial_T);

    break;
  }

  default:
    mooseError("Not implemented yet.");
    break;
  }
}

void
FlowJunction::addMooseObjects()
{
  std::vector<std::string> cv_u(1, FlowModel::VELOCITY);
  std::vector<std::string> cv_pressure(1, FlowModel::PRESSURE);
  std::vector<std::string> cv_rho(1, FlowModel::RHO);
  std::vector<std::string> cv_rhou(1, FlowModel::RHOU);
  std::vector<std::string> cv_rhoE(1, FlowModel::RHOE);

  // Add NumericalFluxUserObject for use with FlowJunction - see
  // HeatStructure.C for another example of adding a UserObject
  {
    InputParameters params = _factory.getValidParams("NumericalFluxUserObject");
    _sim.addUserObject("NumericalFluxUserObject", "numerical_flux", params);
  }

  // add BC terms
  const std::vector<unsigned int> & boundary_ids = getBoundaryIds();
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    std::vector<unsigned int> bnd_id(1, boundary_ids[i]);

    // mass
    {
      InputParameters params = _factory.getValidParams("OneDSpecifiedFluxBC");

      params.set<NonlinearVariableName>("variable") = FlowModel::RHO;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      params.set<std::string>("eqn_name") = "CONTINUITY";
      params.set<std::vector<std::string> >("u")    = cv_u;
      params.set<std::vector<std::string> >("rho")  = cv_rho;
      params.set<std::vector<std::string> >("rhou") = cv_rhou;
      params.set<std::vector<std::string> >("rhoE") = cv_rhoE;
      params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
      params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
      params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);
      params.set<UserObjectName>("numerical_flux") = "numerical_flux";

      _sim.addBoundaryCondition("OneDSpecifiedFluxBC", genName("mass", _id, "_bc"), params);
    }


    // momentum
    {
      InputParameters params = _factory.getValidParams("OneDSpecifiedFluxBC");

      params.set<NonlinearVariableName>("variable") = FlowModel::RHOU;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      params.set<std::string>("eqn_name") = "MOMENTUM";
      params.set<std::vector<std::string> >("u")    = cv_u;
      params.set<std::vector<std::string> >("rho")  = cv_rho;
      params.set<std::vector<std::string> >("rhou") = cv_rhou;
      params.set<std::vector<std::string> >("rhoE") = cv_rhoE;
      params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
      params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
      params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);
      params.set<UserObjectName>("numerical_flux") = "numerical_flux";

      _sim.addBoundaryCondition("OneDSpecifiedFluxBC", genName("mom", _id, "_bc"), params);
    }


    // energy
    if (_model_type == FlowModel::EQ_MODEL_3)
    {
      InputParameters params = _factory.getValidParams("OneDSpecifiedFluxBC");

      params.set<NonlinearVariableName>("variable") = FlowModel::RHOE;
      params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
      params.set<UserObjectName>("eos")  = getParam<UserObjectName>("eos");
      params.set<std::string>("eqn_name") = "ENERGY";
      params.set<std::vector<std::string> >("u")    = cv_u;
      params.set<std::vector<std::string> >("rho")  = cv_rho;
      params.set<std::vector<std::string> >("rhou") = cv_rhou;
      params.set<std::vector<std::string> >("rhoE") = cv_rhoE;
      params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
      params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
      params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);
      params.set<UserObjectName>("numerical_flux") = "numerical_flux";

      _sim.addBoundaryCondition("OneDSpecifiedFluxBC", genName("erg", _id, "_bc"), params);
    }
  }

  // Add the constraints - add the same kernel three times,
  // associating it with a different variable each time.
  {
    // Local variables to help with the loop...
    std::vector<std::string> var_names(3), eqn_names(3), ker_names(3);
    var_names[0] = _junction_rho_name; var_names[1] = _junction_rhou_name; var_names[2] = _junction_rhoE_name;
    eqn_names[0] = "CONTINUITY";       eqn_names[1] = "MOMENTUM";          eqn_names[2] = "ENERGY";
    ker_names[0] = "rho";              ker_names[1] = "rhou";              ker_names[2] = "rhoE";

    for (unsigned v=0; v<3; ++v)
    {
      InputParameters params = _factory.getValidParams("FlowConstraint");

      params.set<NonlinearVariableName>("variable") = var_names[v];
      params.set<std::string>("eqn_name") = eqn_names[v];

      params.set<FlowModel::EModelType>("model_type") = _model_type;
      params.set<std::vector<unsigned int> >("nodes") = _nodes;
      params.set<std::vector<Real> >("areas") = _Areas;
      params.set<std::vector<Real> >("normals") = _normals;
      params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");
      params.set<UserObjectName>("numerical_flux") = "numerical_flux";

      // coupling
      params.set<std::vector<std::string> >("u") = cv_u;
      params.set<std::vector<std::string> >("p") = cv_pressure;
      params.set<std::vector<std::string> >("rho") = cv_rho;
      params.set<std::vector<std::string> >("rhou") = cv_rhou;
      params.set<std::vector<std::string> >("rhoE") = cv_rhoE;
      params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
      params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
      params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);

      // junction parameters
      params.set<Real>("junction_vol")     = _junction_vol;
      params.set<Real>("junction_gravity") = _junction_gravity;
      params.set<Real>("junction_loss")    = _junction_loss;
      params.set<Real>("junction_area")    = _junction_area;

      _sim.addScalarKernel("FlowConstraint", genName(ker_names[v], _id, "_c"), params);
    }
  }



  // Add an AuxScalarKernel for the junction pressure.
  {
    InputParameters params = _factory.getValidParams("ScalarPressureAux");

    // The variable associated with this kernel.
    params.set<AuxVariableName>("variable") = _p_name;

    params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
    params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
    params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);

    // And the equation of state object that the AuxScalarKernel will use.
    params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");

    // Add this kernel to the Simulation object
    _sim.addAuxScalarKernel("ScalarPressureAux", genName("flow", _id, "_p"), params);
  }



  // Add an AuxScalarKernel for the junction temperature.
  {
    InputParameters params = _factory.getValidParams("ScalarTemperatureAux");

    // The variable associated with this kernel.
    params.set<AuxVariableName>("variable") = _T_name;

    params.set<std::vector<std::string> >("junction_rho")  = std::vector<std::string>(1, _junction_rho_name);
    params.set<std::vector<std::string> >("junction_rhou") = std::vector<std::string>(1, _junction_rhou_name);
    params.set<std::vector<std::string> >("junction_rhoE") = std::vector<std::string>(1, _junction_rhoE_name);

    // And the equation of state object that the AuxScalarKernel will use.
    params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");

    // Add this kernel to the Simulation object
    _sim.addAuxScalarKernel("ScalarTemperatureAux", genName("flow", _id, "_T"), params);
  }



  // add PPS for visualization of scalar aux variables
  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep";
  {
    InputParameters params = _factory.getValidParams("PrintScalarVariable");
    params.set<VariableName>("variable") = _p_name;
    params.set<MooseEnum>("output") = "file";
    params.set<MooseEnum>("execute_on") = execute_options;
    _sim.addPostprocessor("PrintScalarVariable", genName(name(), "p"), params);
  }
  {
    InputParameters params = _factory.getValidParams("PrintScalarVariable");
    params.set<VariableName>("variable") = _T_name;
    params.set<MooseEnum>("execute_on") = execute_options;
    params.set<MooseEnum>("output") = "file";
    _sim.addPostprocessor("PrintScalarVariable", genName(name(), "T"), params);
  }
}

std::vector<unsigned int>
FlowJunction::getIDs(std::string /*piece*/)
{
  mooseError("Not implemented yet");
  return std::vector<unsigned int>();
}

std::string
FlowJunction::variableName(std::string /*piece*/)
{
  mooseError("Not implemented yet");
  return std::string();
}
