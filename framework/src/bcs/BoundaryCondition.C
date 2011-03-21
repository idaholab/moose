#include "BoundaryCondition.h"
#include "SubProblem.h"
#include "Variable.h"

template<>
InputParameters validParams<BoundaryCondition>()
{
  InputParameters params = validParams<Object>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  return params;
}


BoundaryCondition::BoundaryCondition(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    FunctionInterface(parameters),
    Moose::TransientInterface(parameters),
    Moose::MaterialPropertyInterface(parameters),
    _problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _var(_problem.getVariable(0, parameters.get<std::string>("variable"))),
    _boundary_id(parameters.get<unsigned int>("_boundary_id")),

    _current_elem(_var.currentElem()),
    _current_side(_var.currentSide()){
}

BoundaryCondition::~BoundaryCondition()
{
}

