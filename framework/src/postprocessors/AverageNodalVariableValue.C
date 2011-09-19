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

#include "AverageNodalVariableValue.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "boundary_info.h"

template<>
InputParameters validParams<AverageNodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeset", "The ID of the node where we monitor");
  return params;
}

AverageNodalVariableValue::AverageNodalVariableValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<std::string>("variable")),
    _nodesetid(parameters.get<unsigned int>("nodeset"))
{
}

Real
AverageNodalVariableValue::getValue()
{
  Real avg = 0;
  std::vector<unsigned int> & nodeset = _mesh.getNodeList(_nodesetid);
  int n = nodeset.size();
  for (int i = 0; i < n; i++)
  {
    if(_mesh.node(nodeset[i]).processor_id() == libMesh::processor_id())
      avg += _problem.getVariable(_tid, _var_name).getNodalValue(_mesh.node(nodeset[i]));
  }

  gatherSum(avg);

  return avg / n;
}
