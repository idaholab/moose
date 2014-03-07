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

#include "CInterfacePosition.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "libmesh/boundary_info.h"

template<>
InputParameters validParams<CInterfacePosition>()
{
  InputParameters params = validParams<NodalProxyMaxValue>();

  params.addParam<Real>("RefVal",0.5,"Variable value used to determine interface position");
  params.addRequiredParam<unsigned int>("direction_index", "The index of the direction the position is measured in");

  return params;
}

CInterfacePosition::CInterfacePosition(const std::string & name, InputParameters parameters) :
  NodalProxyMaxValue(name, parameters),
  _RefVal(getParam<Real>("RefVal")),
    _direction_index(parameters.get<unsigned int>("direction_index")),
    _mesh(_subproblem.mesh())
{
}

Real
CInterfacePosition::computeValue()
{
  Real val = -std::abs(_RefVal - _u[_qp]);

  return val;

}


Real
CInterfacePosition::getValue()
{
  unsigned int node_id = NodalProxyMaxValue::getValue();
  return _mesh.node(node_id)(_direction_index);
}
