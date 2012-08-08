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

#include "SideUserObject.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<SideUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs or names from the mesh where this boundary condition applies");
  return params;
}

SideUserObject::SideUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    Coupleable(parameters, false),
    UserObjectInterface(parameters),
    MooseVariableInterface(parameters, false),
    TransientInterface(parameters, name, "side_user_objects"),
    MaterialPropertyInterface(parameters),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
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
    _grad_u(_var.gradSln()),
    //
    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{}

Real
SideUserObject::computeIntegral()
{
  Real sum = 0;
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}
