#include "FlowJunction.h"
#include "Conversion.h"
#include "Simulation.h"
#include "Pipe.h"
#include "FEProblem.h"
#include <sstream>

#include "FlowJunctionConstraint.h"
#include "OneDMassStaticPandTBC.h"
#include "OneDMomentumCoupledPTBC.h"
#include "OneDEnergyCoupledPTBC.h"

#include "Function.h"
#include "EquationOfState.h"


template<>
InputParameters validParams<FlowJunction>()
{
  InputParameters params = validParams<Joint>();
  params.addRequiredParam<std::vector<std::string> >("inputs", "Inputs of this joint");
  params.addRequiredParam<std::vector<std::string> >("outputs", "Outputs of this joint");

  params.addRequiredParam<std::vector<Real> >("K", "Form loss coefficients");
  params.addRequiredParam<Real>("Area", "Reference area of this branch");
  params.addParam<Real>("Volume", 0.0, "Reference volume of this branch");
  //params.addRequiredParam<Real>("Initial_pressure", "Initial pressure of this branch");
  params.addParam<Real>("initial_P", 1.e5, "Initial pressure of this branch");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  return params;
}


FlowJunction::FlowJunction(const std::string & name, InputParameters params) :
    Joint(name, params),
    _inputs(getParam<std::vector<std::string> >("inputs")),
    _outputs(getParam<std::vector<std::string> >("outputs")),
    _k_coeffs(getParam<std::vector<Real> >("K")),
    _ref_area(getParam<Real>("Area")),
    _ref_volume(getParam<Real>("Volume")),
	_has_initial_P(params.wasSeenInInput("initial_P")),
	_has_initial_T(params.wasSeenInInput("initial_T")),
	_initial_P(_has_initial_P ? getParam<Real>("initial_P") : _sim.getParam<Real>("global_init_P")),
	_initial_T(_has_initial_T ? getParam<Real>("initial_T") : _sim.getParam<Real>("global_init_T"))
{
	_FlowJunction_var_name = genName("variables_of_", _id, "_FlowJunction");
}

FlowJunction::~FlowJunction()
{
}

void
FlowJunction::buildMesh()
{
  // Joint does not create any mesh
  // BUT we will figure out our connectivity here

  // connected inputs
  for (unsigned int i = 0; i < _inputs.size(); ++i)
  {
    std::string comp_name;
    RELAP7::EEndType end_type;
    RELAP7::getConnectionInfo(_inputs[i], comp_name, end_type);

    Component * comp = _sim.getComponentByName(comp_name);
    if (dynamic_cast<PipeBase *>(comp) != NULL)
    {
      PipeBase * pipe = dynamic_cast<PipeBase *>(comp);

      // get the boundary node from the pipe
      Node * nd = pipe->getBoundaryNode(end_type);
      _nodes.push_back(nd->id());
      _Areas.push_back(pipe->getArea());
      //_normals.push_back(1.);
      _normals.push_back(pipe->getBoundaryOutNorm(end_type));
      _bnd_id.push_back(pipe->getBoundaryId(end_type));
    }
  }

  // connected outputs
  for (unsigned int i = 0; i < _outputs.size(); ++i)
  {
    std::string comp_name;
    RELAP7::EEndType end_type;
    RELAP7::getConnectionInfo(_outputs[i], comp_name, end_type);

    Component * comp = _sim.getComponentByName(comp_name);
    if (dynamic_cast<PipeBase *>(comp) != NULL)
    {
      PipeBase * pipe = dynamic_cast<PipeBase *>(comp);

      // get the boundary node from the pipe
      Node * nd = pipe->getBoundaryNode(end_type);
      _nodes.push_back(nd->id());
      _Areas.push_back(pipe->getArea());
      //_normals.push_back(-1.);
      _normals.push_back(pipe->getBoundaryOutNorm(end_type));
      _bnd_id.push_back(pipe->getBoundaryId(end_type));
    }
  }
}

void
FlowJunction::addVariables()
{
  // add scalar variable
  // Note the physical meaning of this scalar variable is the "pressure of the branch, P_Branch"
  const EquationOfState & eos = _sim.getEquationOfState(getParam<UserObjectName>("eos"));

  // Debug
  // std::cout << "initial T " << _initial_T << std::endl;
  // End of debug

  Real _initial_specific_int_energy = 0.;
  if(_model_type == Model::EQ_MODEL_3)
  {
	  _initial_specific_int_energy = eos.invert_eos_internal_energy(_initial_T);
  }

  // Debug
  // std::cout << "initial e " << _initial_specific_int_energy << std::endl;
  // End of debug

  Real _initial_rho = eos.invert_eos_rho(_initial_P, _initial_specific_int_energy);

  if(_model_type == Model::EQ_MODEL_2)
  {
	  //_sim.addVariable(true, _FlowJunction_var_name, FEType(FIRST, SCALAR), 0, 1.);	//FIXME: is '1' the best scaling factor?
	  _sim.addVariable(true, _FlowJunction_var_name, FEType(SECOND, SCALAR), 0, 1.);	//FIXME: is '1' the best scaling factor?
	  std::vector<Real> _initial_conditions(2, 0.);
	  _initial_conditions[0] = _initial_rho;
	  _initial_conditions[1] = 0.;
	  _sim.addScalarInitialCondition(_FlowJunction_var_name, _initial_conditions);
  }
  else if (_model_type == Model::EQ_MODEL_3)
  {
	  //_sim.addVariable(true, _FlowJunction_var_name, FEType(SECOND, SCALAR), 0, 1.);	//FIXME: is '1' the best scaling factor?
	  _sim.addVariable(true, _FlowJunction_var_name, FEType(THIRD, SCALAR), 0, 1.);	//rho_branch, e_branch, net_mass_in_branch
	  std::vector<Real> _initial_conditions(3, 0.);
	  _initial_conditions[0] = _initial_rho;
	  _initial_conditions[1] = 0.;
	  _initial_conditions[2] = _initial_specific_int_energy;
	  _sim.addScalarInitialCondition(_FlowJunction_var_name, _initial_conditions);
  }
  else
	  mooseError("Not implemented yet.");
}

void
FlowJunction::addMooseObjects()
{
  std::vector<std::string> cv_u(1, Model::VELOCITY);
  std::vector<std::string> cv_pressure(1, Model::PRESSURE);
  std::vector<std::string> cv_rho(1, Model::RHO);
  std::vector<std::string> cv_rhou(1, Model::RHOU);
  std::vector<std::string> cv_rhoE(1, Model::RHOE);

  std::vector<std::string> cv_flowjunction_var(1, _FlowJunction_var_name);

  //add constraint
  {
    InputParameters params = validParams<FlowJunctionConstraint>();
    params.set<std::string>("variable") = _FlowJunction_var_name;
    params.set<std::string>("ced_variable") = Model::RHO;

    params.set<std::vector<unsigned int> >("nodes") = _nodes;
    params.set<std::vector<Real> >("areas") = _Areas;
    params.set<Real>("Volume") = _ref_volume;
    params.set<std::vector<Real> >("normals") = _normals;
    params.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");

    params.set<std::vector<Real> >("K") = _k_coeffs;
    params.set<Real>("Area") = _ref_area;

    // coupling variables
    params.set<std::vector<std::string> >("u") = cv_u;
    params.set<std::vector<std::string> >("rhou") = cv_rhou;
    if(_model_type == Model::EQ_MODEL_3)
    	params.set<std::vector<std::string> >("rhoE") = cv_rhoE;

    std::string mon = genName(name(), _id, "_flowJunction");
    _sim.addScalarKernel("FlowJunctionConstraint", mon, params);
    connectObject("", mon);
  }

  //add BC's
  for (unsigned int i = 0; i < _bnd_id.size(); i++)
  {
	  std::vector<unsigned int> bnd_id(1, _bnd_id[i]);
	  // adding mass equation BC
	  {
	    InputParameters params = validParams<OneDMassStaticPandTBC>();
	    params.set<NonlinearVariableName>("variable") = Model::RHO;
	    params.set<std::vector<unsigned int> >("boundary") = bnd_id;
	    // coupling
	    params.set<std::vector<std::string> >("u") = cv_u;
	    params.set<std::vector<std::string> >("rhou") = cv_rhou;

	    _sim.addBoundaryCondition("OneDMassStaticPandTBC", genName("bc", _id * 1000 + i, "rho"), params);
	  }

	  // Debug
	  /*
	  {
	    InputParameters params = validParams<OneDMomentumCoupledPTBC>();
	    params.set<std::string>("variable") = Model::RHOU;
	    params.set<std::vector<unsigned int> >("boundary") = bnd_id;
	    // coupling
	    params.set<std::vector<std::string> >("u") = cv_u;
	    params.set<std::vector<std::string> >("rho") = cv_rho;
	    // additional params
	    //params.set<Real>("p_in") = _p_bc;
	    //params.set<Real>("T_in") = _T_bc;
            pars.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");

	    params.set<std::vector<std::string> >("FlowJunctionVar") = cv_flowjunction_var;
	    params.set<Real>("k_coeff") = _k_coeffs[i];
	    params.set<Real>("k_coeff_reverse") = _k_coeffs[i];	//FIXME

	    _sim.addBoundaryCondition("OneDMomentumCoupledPTBC", genName("bc", _id * 1000 + i, "rhou"), params);
	  }
	  */
	  // End of debug
  }
}
