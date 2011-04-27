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

#include "NearestNodeValueAux.h"

#include "MooseMesh.h"
#include "SystemBase.h"

template<>
InputParameters validParams<NearestNodeValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to get the value from.");
  params.addRequiredCoupledVar("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

NearestNodeValueAux::NearestNodeValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _nearest_node(getNearestNodeLocator(parameters.get<unsigned int>("paired_boundary"), getParam<std::vector<unsigned int> >("boundary")[0])),
    _serialized_solution(_nl_sys.currentSolution()),
    _paired_variable(coupled("paired_variable"))
{
  if(getParam<std::vector<unsigned int> >("boundary").size() > 1)
    mooseError("NearestNodeValueAux can only be used with one boundary at a time!");
}

Real
NearestNodeValueAux::computeValue()
{
  // Assumes the variable you are coupling to is from the nonlinear system for now.
  const Node * nearest = _nearest_node.nearestNode(_current_node->id());
  mooseAssert(nearest != NULL, "I do not have the nearest node for you");
  long int dof_number = nearest->dof_number(_nl_sys.number(), _paired_variable, 0);

  return (*_serialized_solution)(dof_number);
}
