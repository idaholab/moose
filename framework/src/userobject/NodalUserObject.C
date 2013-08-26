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


  /// \TODO: The default for both Boundary and Block Restrictable should be ANY...

  // NodalUserObjects can be restricted to either Nodesets or Domains
  std::vector<BoundaryName> everywhere(1);
  everywhere[0] = "ANY_BOUNDARY_ID";
  params.addParam<std::vector<BoundaryName> >("boundary", everywhere, "boundary ID or name where the postprocessor works");

  std::vector<SubdomainName> block_everywhere(1);
  block_everywhere[0] = "ANY_BLOCK_ID";
  params.set<std::vector<SubdomainName> >("block") = block_everywhere;

  return params;
}

NodalUserObject::NodalUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters),
    UserObjectInterface(parameters),
    Coupleable(parameters, true),
    ScalarCoupleable(parameters),
    MooseVariableDependencyInterface(),
    TransientInterface(parameters, name, "nodal_user_objects"),
    PostprocessorInterface(parameters),
    _qp(0),
    _current_node(_assembly.node()),
    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid])
{
}
