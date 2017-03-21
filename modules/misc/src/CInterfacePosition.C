/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CInterfacePosition.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "libmesh/boundary_info.h"

template <>
InputParameters
validParams<CInterfacePosition>()
{
  InputParameters params = validParams<NodalProxyMaxValue>();

  params.addParam<Real>("RefVal", 0.5, "Variable value used to determine interface position");
  params.addRequiredParam<unsigned int>("direction_index",
                                        "The index of the direction the position is measured in");

  return params;
}

CInterfacePosition::CInterfacePosition(const InputParameters & parameters)
  : NodalProxyMaxValue(parameters),
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
  return _mesh.nodeRef(node_id)(_direction_index);
}
