#include "SolidWall.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Pipe.h"
#include "Factory.h"

template<>
InputParameters validParams<SolidWall>()
{
  InputParameters params = validParams<BoundaryBase>();
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
SolidWall::addMooseObjects1Phase()
{
  std::vector<unsigned int> bnd_id(1, getBoundaryId());
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModel::RHOUA;
    params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName(name(), "rhou"), params);
  }
}

void
SolidWall::addMooseObjects2Phase()
{
  std::vector<unsigned int> bnd_id(1, getBoundaryId());
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModel::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName(name(), "alpha_rhou_A_liquid"), params);
  }
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModel::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<unsigned int> >("r7:boundary") = bnd_id;
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName(name(), "alpha_rhou_A_vapor"), params);
  }
}

void
SolidWall::addMooseObjects()
{
  if (_model_type == FlowModel::EQ_MODEL_2 ||
      _model_type == FlowModel::EQ_MODEL_3 ||
      _model_type == FlowModel::EQ_MODEL_HEM)
    addMooseObjects1Phase();
  else if (_model_type == FlowModel::EQ_MODEL_7)
    addMooseObjects2Phase();
}
