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

#include "Moose.h"
#include "NodalUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<NodalUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addRequiredParam<std::vector<VariableName> >("variable", "The name of the variable that this postprocessor operates on");
  std::vector<BoundaryName> everywhere(1);
  everywhere[0] = "ANY_BOUNDARY_ID";
  params.addParam<std::vector<BoundaryName> >("boundary", everywhere, "boundary ID or name where the postprocessor works");
  return params;
}

NodalUserObject::NodalUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    Coupleable(parameters, true),
    UserObjectInterface(parameters),
    MooseVariableInterface(parameters, true),
    TransientInterface(parameters, name, "nodal_user_objects"),
    MaterialPropertyInterface(parameters),
    _var(_subproblem.getVariable(_tid, parameters.get<std::vector<VariableName> >("variable")[0])),
    _boundaries(parameters.get<std::vector<BoundaryName> >("boundary")),
    _qp(0),
    _current_node(_var.node()),
    _u(_var.nodalSln()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid])
{
  std::vector<VariableName> vars = getParam<std::vector<VariableName> >("variable");
  _vars.resize(vars.size());

  // initialize our vector of variable pointers
  for (unsigned int i=0; i<vars.size(); ++i)
    _vars[i] = &_subproblem.getVariable(0, vars[i]);
}
