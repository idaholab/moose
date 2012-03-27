#include "SolidWall.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Pipe.h"
//
#include "OneDMassSolidWallBC.h"
#include "OneDMomentumSolidWallBC.h"
#include "OneDEnergySolidWallBC.h"

template<>
InputParameters validParams<SolidWall>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>("input", "Pipe name");
  return params;
}


SolidWall::SolidWall(const std::string & name, InputParameters params) :
    Component(name, params),
    _input(getParam<std::string>("input"))
{
}

SolidWall::~SolidWall()
{
}

void
SolidWall::buildMesh()
{
  // NOTE: we are not building any mesh here, we just extract some mesh related info

  // extract boundary id so we can set up our BCs
  std::string comp_name;
  RELAP7::EEndType end_type;
  RELAP7::getConnectionInfo(_input, comp_name, end_type);

  Component * comp = _sim.getComponentByName(comp_name);
  if (dynamic_cast<PipeBase *>(comp) != NULL)
  {
    PipeBase * pipe = dynamic_cast<PipeBase *>(comp);
    _boundary_id = pipe->getBoundaryId(end_type);
  }
  else
  {
    mooseError("Component " << comp->name() << " is not of a pipe type");
  }
}

void
SolidWall::addVariables()
{
}

void
SolidWall::addMooseObjects()
{
  // coupling "vectors"
  std::vector<std::string> cv_p(1, Model::PRESSURE);
  std::vector<std::string> cv_rho(1, Model::RHO);
  // boundary id
  std::vector<unsigned int> bnd_id(1, _boundary_id);
  {
    InputParameters params = validParams<OneDMassSolidWallBC>();
    params.set<std::string>("variable") = Model::RHO;
    params.set<std::vector<unsigned int> >("boundary") = bnd_id;
    _sim.addBoundaryCondition("OneDMassSolidWallBC", genName("bc", _id, "rho"), params);
  }
  {
    InputParameters params = validParams<OneDMomentumSolidWallBC>();
    params.set<std::string>("variable") = Model::RHOU;
    params.set<std::vector<unsigned int> >("boundary") = bnd_id;
    params.set<std::string>("eos_function") = Model::EOS_FUNCTION;
    // coupling
    params.set<std::vector<std::string> >("p") = cv_p;
    params.set<std::vector<std::string> >("rho") = cv_rho;
    _sim.addBoundaryCondition("OneDMomentumSolidWallBC", genName("bc", _id, "rhou"), params);
  }

  if (_model_type == Model::EQ_MODEL_3)
  {
    InputParameters params = validParams<OneDEnergySolidWallBC>();
    params.set<std::string>("variable") = Model::RHOE;
    params.set<std::vector<unsigned int> >("boundary") = bnd_id;
    params.set<std::string>("eos_function") = Model::EOS_FUNCTION;

    // There is no coupling for the energy equation solid wall BC.
    // It simply returns zero...

    _sim.addBoundaryCondition("OneDEnergySolidWallBC", genName("bc", _id, "rhoE"), params);
  }
}
