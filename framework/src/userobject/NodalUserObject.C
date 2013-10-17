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

#include "NodalUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

template<>
InputParameters validParams<NodalUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<RandomInterface>();

  /// \TODO: The default for both Boundary and Block Restrictable should be ANY...
  return params;
}

NodalUserObject::NodalUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    BlockRestrictable(name, parameters),
    BoundaryRestrictable(name, parameters),
    MaterialPropertyInterface(parameters),
    UserObjectInterface(parameters),
    Coupleable(parameters, true),
    ScalarCoupleable(parameters),
    MooseVariableDependencyInterface(),
    TransientInterface(parameters, name, "nodal_user_objects"),
    PostprocessorInterface(parameters),
    RandomInterface(parameters, _fe_problem, _tid, true),
    _qp(0),
    _current_node(_assembly.node()),
    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid])
{
}
