#include "SolidWall.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Pipe.h"
#include "Factory.h"

template<>
InputParameters validParams<SolidWall>()
{
  InputParameters params = validParams<BoundaryBase>();
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  return params;
}


SolidWall::SolidWall(const std::string & name, InputParameters params) :
    BoundaryBase(name, params)
{
}

SolidWall::~SolidWall()
{
}

void
SolidWall::addVariables()
{
}

void
SolidWall::addMooseObjects()
{
  std::vector<unsigned int> bnd_id(1, getBoundaryId());
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModel::RHOU;
    params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName("bc", _id, "rho"), params);
  }
}
