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
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this postprocessor operates on");
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
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _boundaries(parameters.get<std::vector<BoundaryName> >("boundary")),
    _qp(0),
    _current_node(_var.node()),
    _u(_var.nodalSln()),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid])
{
}
