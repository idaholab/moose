#include "PipeBase.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "R7Conversion.h"
#include "Factory.h"

#include "edge_edge2.h"
#include "fe_type.h"


const std::string PipeBase::_type("pipe");


template<>
InputParameters validParams<PipeBase>()
{
  InputParameters params = validParams<Component>();
  params.addParam("component_type", PipeBase::_type, "The type of the component");
  //Input parameters [NO] default values should be given.
  params.addRequiredParam<std::vector<Real> >("position", "Origin (start) of the pipe");

  std::vector<Real> zr(LIBMESH_DIM, 0.);
  params.addParam<std::vector<Real> >("offset", zr, "Offset of the origin for mesh generation");
  params.addRequiredParam<std::vector<Real> >("orientation", "Orientation vector of the pipe");
  params.addRequiredParam<Real>("length", "Length of the pipe");
  params.addRequiredParam<unsigned int>("n_elems", "number of element in this pipe");
  params.addRequiredParam<Real>("A", "Area of the pipe");
  params.addRequiredParam<UserObjectName>("eos", "The name of EOS to use");

  //Input parameters default values could be given.
  params.addParam<Real>("f", 0.0, "friction");
  params.addParam<Real>("Hw", 0.0, "Convective heat transfer coefficient");
  params.addParam<Real>("Tw", 400, "Wall temperature");


  params.addParam<Real>("initial_P", 0., "Initial pressure in the pipe");
  params.addParam<Real>("initial_V", 0., "Initial velocity in the pipe");
  params.addParam<Real>("initial_T", 300., "Initial temperature in the pipe");

  return params;
}


PipeBase::PipeBase(const std::string & name, InputParameters params) :
    Component(name, params),
    Model(params),
    _position(toPoint(getParam<std::vector<Real> >("position"))),
    _offset(toPoint(getParam<std::vector<Real> >("offset"))),
    _length(getParam<Real>("length")),
    _n_elems(getParam<unsigned int>("n_elems")),
    _A(getParam<Real>("A")),
    _has_f(params.wasSeenInInput("f")),
    _f(getParam<Real>("f")),
    _Hw(getParam<Real>("Hw")),
    _Tw(getParam<Real>("Tw")),
    _has_initial_P(params.wasSeenInInput("initial_P")),
    _has_initial_V(params.wasSeenInInput("initial_V")),
    _has_initial_T(params.wasSeenInInput("initial_T")),
    _initial_P(_has_initial_P ? getParam<Real>("initial_P") : 0.),
    _initial_V(_has_initial_V ? getParam<Real>("initial_V") : 0.),
    _initial_T(_has_initial_T ? getParam<Real>("initial_T") : 300.)
{
  const std::vector<Real> & dir = getParam<std::vector<Real> >("orientation");
  _dir = VectorValue<Real>(dir[0], dir[1], dir[2]);

  //compute the gravity along the pipe direction.
  RealVectorValue gravity_vector = _sim.getParam<VectorValue<Real> >("gravity");
  _gx = _dir * gravity_vector / _dir.size();
}

PipeBase::~PipeBase()
{
}

Node *
PipeBase::getBoundaryNode(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, Node *>::iterator it = _bnd_nodes.find(id);
  if (it != _bnd_nodes.end())
    return it->second;
  else
    return NULL;
}

unsigned int
PipeBase::getBoundaryId(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, unsigned int>::iterator it = _bnd_ids.find(id);
  if (it != _bnd_ids.end())
    return it->second;
  else
    mooseError("PipeBase " << name() << " does not have this type of end defined.");
}

int
PipeBase::getBoundaryOutNorm(RELAP7::EEndType id)
{
  std::map<RELAP7::EEndType, int>::iterator it = _bnd_out_norm.find(id);
  if (it != _bnd_out_norm.end())
    return it->second;
  else
    mooseError("PipeBase " << name() << " does not have this type of end defined.");
}

void
PipeBase::buildMesh()
{
  // points
  Real delta_t = _length / _n_elems;
  Point p(0, 0, 0);                      // origin
  p += _offset;
  for (unsigned int i = 0; i <= _n_elems; i++)
  {
    const Node * nd = _mesh._mesh.add_point(p);
    node_ids.push_back(nd->id());
    p(0) += delta_t;
  }

  // elems
  _subdomain_id = getNextSubdomainId();
  unsigned int bc_id_in = getNextBCId();
  unsigned int bc_id_out = getNextBCId();
  for (unsigned int i = 0; i < _n_elems; i++)
  {
    Elem * elem = _mesh._mesh.add_elem (new Edge2);
    elem->subdomain_id() = _subdomain_id;
    elem->set_node(0) = _mesh._mesh.node_ptr(node_ids[i]);
    elem->set_node(1) = _mesh._mesh.node_ptr(node_ids[i+1]);
    elem_ids.push_back(elem->id());

    // BCs
    if (i == 0)
    {
      _bnd_ids[RELAP7::IN] = bc_id_in;
      _mesh._mesh.boundary_info->add_side(elem, 0, bc_id_in);
      _bnd_nodes[RELAP7::IN] = elem->get_node(0);                // first 0 is local bnd_id that Joints will use for connecting
      _bnd_out_norm[RELAP7::IN] = -1;
    }
    if (i == (_n_elems - 1))
    {
      _bnd_ids[RELAP7::OUT] = bc_id_out;
      _mesh._mesh.boundary_info->add_side(elem, 1, bc_id_out);
      _bnd_nodes[RELAP7::OUT] = elem->get_node(1);               // first 1 is local bnd_id that Joints will use for connecting
      _bnd_out_norm[RELAP7::OUT] = 1;
    }
 }
}

void
PipeBase::addVariables()
{
  Model::addVariables(_subdomain_id);

  const EquationOfState & eos = _sim.getEquationOfState(getParam<UserObjectName>("eos"));

  Real initial_P = _sim.getParam<Real>("global_init_P");
  Real initial_V = _sim.getParam<Real>("global_init_V");
  Real initial_T = _sim.getParam<Real>("global_init_T");
  if(_has_initial_P)
	  initial_P = _initial_P;			//replace by local initial conditions
  if(_has_initial_V)
	  initial_V = _initial_V;
  if(_has_initial_T)
	  initial_T = _initial_T;

  //Model::addInitialConditions(_subdomain_id, initial_P, initial_V, initial_T);


  Real initial_rho = 0.;
  Real initial_rhou = 0.;
  Real initial_e = 0.;
  Real initial_rhoE = 0.;
  Real initial_enthalpy = 0.;
  if(_model_type == EQ_MODEL_2)
  {
	  initial_rho = eos.invert_eos_rho(initial_P);
	  initial_rhou = initial_rho * initial_V;
  }
  else if(_model_type == EQ_MODEL_3)
  {
	  initial_e = eos.invert_eos_internal_energy(initial_T);
	  initial_rho = eos.invert_eos_rho(initial_P, initial_e);
	  initial_rhou = initial_rho * initial_V;
	  initial_rhoE = initial_rho * initial_e + initial_rho * 0.5 * initial_V * initial_V;
	  initial_enthalpy = (initial_rhoE + initial_P) / initial_rho;
  }
  else
	  mooseError("wrong model type");

  Model::addInitialConditions(_subdomain_id, initial_P, initial_V, initial_rho, initial_rhou, initial_T, initial_rhoE, initial_enthalpy);
}

void
PipeBase::addMooseObjects()
{
  std::vector<unsigned int> blocks(1, _subdomain_id);

  InputParameters pars = emptyInputParameters();
  pars.set<std::vector<unsigned int> >("block") = blocks;

  pars.set<Real>("gx") = _gx;
  pars.set<Real>("dh") = _Dh;
  pars.set<Real>("aw") = _aw;
  if (_has_f)
    pars.set<Real>("f") = _f;
  pars.set<Real>("Hw") = _Hw;
  pars.set<Real>("Tw") = _Tw;
  pars.set<UserObjectName>("eos") = getParam<UserObjectName>("eos");

  Model::addKernels(pars);

  if (_model_type == EQ_MODEL_3)
  {
    // for 3 eqn model, add wall heating term

    std::vector<std::string> cv_rho(1, RHO);
    std::vector<std::string> cv_temperature(1, TEMPERATURE);

    InputParameters params = Factory::instance()->getValidParams("OneDEnergyWallHeating");
    params.set<NonlinearVariableName>("variable") = Model::RHOE;
    params.set<std::vector<unsigned int> >("block") = blocks;

    params.set<Real>("Hw") = _Hw;
    params.set<Real>("aw") = _aw;
    params.set<Real>("Tw") = _Tw;

    params.set<std::vector<std::string> >("rho") = cv_rho;
    params.set<std::vector<std::string> >("temperature") = cv_temperature;

    std::string mon = genName(name(), _id, "_pipe");
    _sim.addKernel("OneDEnergyWallHeating", mon, params);
    connectObject("", mon);
  }
}

std::vector<unsigned int>
PipeBase::getIDs(std::string part)
{
  std::vector<unsigned int> ids;

  if(part == "PIPE")
    ids.push_back(_subdomain_id);
  else
    mooseError(part + " is not a valid part of PipeBase!");

  return ids;
}

std::string
PipeBase::variableName(std::string part)
{
  if(part == "FLUID_TEMP")
    return Model::TEMPERATURE;
  else
    mooseError(part + " is not a valid part of PipeBase!");
}
