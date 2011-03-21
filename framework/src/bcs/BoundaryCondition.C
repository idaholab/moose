#include "BoundaryCondition.h"
#include "SubProblem.h"
#include "System.h"
#include "Variable.h"

template<>
InputParameters validParams<BoundaryCondition>()
{
  InputParameters params = validParams<Object>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  return params;
}


BoundaryCondition::BoundaryCondition(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    Moose::Coupleable(parameters),
    FunctionInterface(parameters),
    Moose::TransientInterface(parameters),
    Moose::MaterialPropertyInterface(parameters),
    Moose::PostprocessorInterface(parameters),
    _problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _sys(*parameters.get<Moose::System *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _dim(_problem.mesh().dimension()),
    _boundary_id(parameters.get<unsigned int>("_boundary_id")),

    _current_elem(_var.currentElem()),
    _current_side(_var.currentSide()),
    _normals(_var.normals()),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}

BoundaryCondition::~BoundaryCondition()
{
}

