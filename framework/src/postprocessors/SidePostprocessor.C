#include "SidePostprocessor.h"
#include "Problem.h"

template<>
InputParameters validParams<SidePostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  return params;
}

SidePostprocessor::SidePostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _boundary_id(parameters.get<unsigned int>("_boundary_id")),
    _q_point(_var.qpoints()),
    _normals(_var.normals()),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{}

