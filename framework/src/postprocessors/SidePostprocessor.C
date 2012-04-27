/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SidePostprocessor.h"
#include "SubProblem.h"

template<>
InputParameters validParams<SidePostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs or names from the mesh where this boundary condition applies");
  return params;
}

SidePostprocessor::SidePostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    UserObjectInterface(parameters),
    MaterialPropertyInterface(parameters),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _boundaries(parameters.get<std::vector<BoundaryName> >("boundary")),
    _q_point(_subproblem.pointsFace(_tid)),
    _qrule(_subproblem.qRuleFace(_tid)),
    _JxW(_subproblem.JxWFace(_tid)),
    _coord(_subproblem.coords(_tid)),
    _normals(_var.normals()),
    _current_elem(_subproblem.elem(_tid)),
    _current_side_elem(_subproblem.sideElem(_tid)),
    _current_side_volume(_subproblem.sideElemVolume(_tid)),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{}

Real
SidePostprocessor::computeIntegral()
{
  Real sum = 0;
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}
