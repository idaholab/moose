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
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs or names from the mesh where this boundary condition applies");
  return params;
}

SideUserObject::SideUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "side_user_objects"),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
//    _var(_subproblem.getVariable(_tid, parameters.get<std::vector<VariableName> >("variable")[0])),
    _boundaries(parameters.get<std::vector<BoundaryName> >("boundary")),
    _q_point(_subproblem.pointsFace(_tid)),
    _qrule(_subproblem.qRuleFace(_tid)),
    _JxW(_subproblem.JxWFace(_tid)),
    _coord(_subproblem.coords(_tid)),
    _normals(_subproblem.assembly(_tid).normals()),
    _current_elem(_subproblem.elem(_tid)),
    _current_side(_subproblem.side(_tid)),
    _current_side_elem(_subproblem.sideElem(_tid)),
    _current_side_volume(_subproblem.sideElemVolume(_tid)),
    //  _u(_var.sln()),
//    _grad_u(_var.gradSln()),
    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
//  std::vector<VariableName> vars = getParam<std::vector<VariableName> >("variable");
//  _vars.resize(vars.size());
//
//  // initialize our vector of variable pointers
//  for (unsigned int i=0; i<vars.size(); ++i)
//    _vars[i] = &_subproblem.getVariable(0, vars[i]);

  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}
