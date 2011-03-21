#include "SidePostprocessor.h"
#include "SubProblem.h"

template<>
InputParameters validParams<SidePostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  return params;
}

SidePostprocessor::SidePostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _boundary_id(parameters.get<unsigned int>("_boundary_id")),
    _q_point(_problem.pointsFace(_tid)),
    _qrule(_problem.qRuleFace(_tid)),
    _JxW(_problem.JxWFace(_tid)),
    _normals(_var.normals()),
    _current_elem(_problem.elem(_tid)),
    _current_side_elem(_problem.sideElem(_tid)),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{}

Real
SidePostprocessor::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  return sum;
}
